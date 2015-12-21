/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/blockchain/fetch_block.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <system_error>
#include <bitcoin/blockchain/blockchain.hpp>

namespace libbitcoin {
namespace chain {

using std::placeholders::_1;
using std::placeholders::_2;

// TODO: split into header.
// This class is used only locally.
class block_fetcher
  : public std::enable_shared_from_this<block_fetcher>
{
public:
    block_fetcher(blockchain& chain)
      : blockchain_(chain)
    {
    }

    template <typename BlockIndex>
    void start(const BlockIndex& index, block_fetch_handler handler)
    {
        // Create the block.
        const auto block = std::make_shared<block_type>();

        blockchain_.fetch_block_header(index,
            std::bind(&block_fetcher::handle_fetch_header,
                shared_from_this(), _1, _2, block, handler));
    }

private:
    typedef std::shared_ptr<block_type> block_ptr;

    void handle_fetch_header(const std::error_code& ec,
        const block_header_type& header, block_ptr block,
        block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Set the block header.
        block->header = header;
        const auto hash = hash_block_header(header);

        blockchain_.fetch_block_transaction_hashes(hash,
            std::bind(&block_fetcher::fetch_transactions,
                shared_from_this(), _1, _2, block, handler));
    }

    void fetch_transactions(const std::error_code& ec,
        const hash_list& hashes, block_ptr block, block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Set the block transaction size.
        const auto count = hashes.size();
        block->transactions.resize(count);

        // This will be called exactly once by the synchronizer.
        const auto completion_handler =
            std::bind(&block_fetcher::handle_complete,
                shared_from_this(), _1, _2, handler);

        // Synchronize transaction fetch calls to one completion call.
        const auto complete = synchronizer<block_fetch_handler>(
            completion_handler, count, "block_fetcher");

        // blockchain::fetch_transaction is thread safe.
        size_t index = 0;
        for (const auto& hash: hashes)
            blockchain_.fetch_transaction(hash,
                std::bind(&block_fetcher::handle_fetch_transaction,
                    shared_from_this(), _1, _2, index++, block, complete));
    }

    void handle_fetch_transaction(const std::error_code& ec,
        const transaction_type& transaction, size_t index, block_ptr block,
        block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Critical Section
        ///////////////////////////////////////////////////////////////////////
        // A vector write cannot be executed concurrently with read|write.
        if (true)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Set a transaction into the block.
            block->transactions[index] = transaction;
        }
        ///////////////////////////////////////////////////////////////////////

        handler(error::success, block);
    }

    // If ec success then there is no possibility that block is being written.
    void handle_complete(const std::error_code& ec, block_ptr block,
        block_fetch_handler handler)
    {
        if (ec)
            handler(ec, nullptr);
        else
            handler(error::success, block);
    }

    std::mutex mutex_;
    blockchain& blockchain_;
};

void fetch_block(blockchain& chain, size_t height,
    block_fetch_handler handle_fetch)
{
    const auto fetcher = std::make_shared<block_fetcher>(chain);
    fetcher->start(height, handle_fetch);
}

void fetch_block(blockchain& chain, const hash_digest& hash,
    block_fetch_handler handle_fetch)
{
    const auto fetcher = std::make_shared<block_fetcher>(chain);
    fetcher->start(hash, handle_fetch);
}

} // namespace chain
} // namespace libbitcoin
