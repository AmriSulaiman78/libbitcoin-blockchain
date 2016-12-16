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
#ifndef LIBBITCOIN_BLOCKCHAIN_ORGANIZER_HPP
#define LIBBITCOIN_BLOCKCHAIN_ORGANIZER_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/interface/fast_chain.hpp>
#include <bitcoin/blockchain/interface/safe_chain.hpp>
#include <bitcoin/blockchain/pools/block_pool.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/validation/fork.hpp>
#include <bitcoin/blockchain/validation/validate_block.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is not thread safe.
/// Organises blocks via the orphan pool to the blockchain.
class BCB_API organizer
{
public:
    typedef handle0 result_handler;
    typedef std::shared_ptr<organizer> ptr;
    typedef safe_chain::reorganize_handler reorganize_handler;
    typedef resubscriber<code, size_t, block_const_ptr_list_const_ptr,
        block_const_ptr_list_const_ptr> reorganize_subscriber;

    /// Construct an instance.
    organizer(threadpool& thread_pool, fast_chain& chain,
        block_pool& block_pool, const settings& settings);

    virtual bool start();
    virtual bool stop();

    virtual void organize(block_const_ptr block, result_handler handler);
    virtual void subscribe_reorganize(reorganize_handler&& handler);

protected:
    virtual bool stopped() const;

private:
    // Utility.
    fork::ptr find_connected_fork(block_const_ptr block);

    // Organize sequence.
    void complete(const code& ec, scope_lock::ptr lock,
        result_handler handler);

    // Verify sub-sequence.
    void verify(fork::ptr fork, size_t index, result_handler handler);
    void handle_accept(const code& ec, fork::ptr fork, size_t index,
        result_handler handler);
    void handle_connect(const code& ec, fork::ptr fork, size_t index,
        result_handler handler);
    void organized(fork::ptr fork, result_handler handler);
    void handle_reorganized(const code& ec, fork::const_ptr fork,
        block_const_ptr_list_ptr outgoing_blocks, result_handler handler);

    // Subscription.
    void notify_reorganize(size_t fork_height,
        block_const_ptr_list_const_ptr fork,
        block_const_ptr_list_const_ptr original);

    // This is protected by mutex.
    fast_chain& fast_chain_;
    mutable upgrade_mutex mutex_;

    // These are thread safe.
    std::atomic<bool> stopped_;
    const bool flush_reorganizations_;
    block_pool& block_pool_;
    threadpool priority_pool_;
    validate_block validator_;
    reorganize_subscriber::ptr subscriber_;
    mutable dispatcher dispatch_;
    mutable dispatcher priority_dispatch_;

};

} // namespace blockchain
} // namespace libbitcoin

#endif
