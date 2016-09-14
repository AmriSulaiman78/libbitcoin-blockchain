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
#include <bitcoin/blockchain/validate_block_impl.hpp>

#include <algorithm>
#include <cstddef>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/block_detail.hpp>
#include <bitcoin/blockchain/simple_chain.hpp>

namespace libbitcoin {
namespace blockchain {
    
// Value used to define median time past.
static constexpr size_t median_time_past_blocks = 11;

// Value used to define retargeting range constraint.
static constexpr uint64_t retargeting_factor = 4;

// Aim for blocks every 10 mins (600 seconds).
static constexpr uint64_t target_spacing_seconds = 10 * 60;

// Target readjustment every 2 weeks (1209600 seconds).
static constexpr uint64_t target_timespan_seconds = 2 * 7 * 24 * 60 * 60;

// The target number of blocks for 2 weeks of work (2016 blocks).
static constexpr uint64_t retargeting_interval = target_timespan_seconds /
    target_spacing_seconds;

validate_block_impl::validate_block_impl(simple_chain& chain,
    size_t fork_height, const block_detail::list& orphan_chain,
    size_t orphan_index, const chain::block& block, size_t height,
    bool testnet, const config::checkpoint::list& checks,
    stopped_callback stopped)
  : validate_block(block, height, testnet, checks, stopped),
    chain_(chain),
    height_(height),
    fork_height_(fork_height),
    orphan_index_(orphan_index),
    orphan_chain_(orphan_chain)
{
    BITCOIN_ASSERT(height_ != 0);
    BITCOIN_ASSERT(orphan_index_ < orphan_chain_.size());
}

// Unsafe Interface
// ----------------------------------------------------------------------------

// TODO: deprecated as unsafe, use of fetch_block ignores error code.
uint32_t validate_block_impl::previous_block_bits() const
{
    // Read block header (top - 1) and return bits
    return fetch_block(height_ - 1).bits;
}

// TODO: cache the result so that only one value must be read per block.
// TODO: deprecated as unsafe, use of fetch_block ignores error code.
validate_block::versions validate_block_impl::preceding_block_versions(
    size_t maximum) const
{
    // 1000 previous versions maximum sample.
    // 950 previous versions minimum required for enforcement.
    // 750 previous versions minimum required for activation.
    const auto size = std::min(maximum, height_);

    // Read block (top - 1) through (top - 1000) and return version vector.
    versions result;
    for (size_t index = 0; index < size; ++index)
    {
        const auto version = fetch_block(height_ - index - 1).version;

        // Some blocks have high versions, see block #390777.
        static const auto maximum = static_cast<uint32_t>(max_uint8);
        const auto normal = std::min(version, maximum);
        result.push_back(static_cast<uint8_t>(normal));
    }

    return result;
}

// TODO: deprecated as unsafe, use of fetch_block ignores error code.
uint64_t validate_block_impl::actual_time_span(size_t interval) const
{
    BITCOIN_ASSERT(height_ > 0 && height_ >= interval);

    // height - interval and height - 1, return time difference
    return fetch_block(height_ - 1).timestamp -
        fetch_block(height_ - interval).timestamp;
}

// TODO: deprecated as unsafe, use of fetch_block ignores error code.
uint64_t validate_block_impl::median_time_past() const
{
    // Read last 11 (or height if height < 11) block times into array.
    const auto count = std::min(height_, median_time_past_blocks);

    std::vector<uint64_t> times;
    for (size_t i = 0; i < count; ++i)
        times.push_back(fetch_block(height_ - i - 1).timestamp);

    // Sort and select middle (median) value from the array.
    std::sort(times.begin(), times.end());
    return times.empty() ? 0 : times[times.size() / 2];
}

// TODO: deprecated as unsafe, ignores error code.
uint32_t validate_block_impl::work_required(const chain::block& block, 
    bool is_testnet) const
{
    if (height_ == 0)
        return max_work_bits;

    const auto is_retarget_height = [](size_t height)
    {
        return height % retargeting_interval == 0;
    };

    if (is_retarget_height(height_))
    {
        // This is the total time it took for the last 2016 blocks.
        const auto actual = actual_time_span(retargeting_interval);

        // Now constrain the time between an upper and lower bound.
        const auto constrained = bc::range_constrain(actual,
            target_timespan_seconds / retargeting_factor,
            target_timespan_seconds * retargeting_factor);

        // TODO: technically this set_compact can fail.
        hash_number retarget;
        retarget.set_compact(previous_block_bits());
        retarget *= constrained;
        retarget /= target_timespan_seconds;

        // TODO: This should be statically-initialized.
        hash_number maximum;
        maximum.set_compact(max_work_bits);

        if (retarget > maximum)
            retarget = maximum;

        return retarget.compact();
    }

    if (!is_testnet)
        return previous_block_bits();

    // TESTNET ONLY

    const auto max_time_gap = fetch_block(height_ - 1).timestamp + 2 * 
        target_spacing_seconds;

    if (block.header.timestamp > max_time_gap)
        return max_work_bits;

    chain::header previous_block;
    auto previous_height = height_;

    // TODO: this is very suboptimal, cache the set of change points.
    // Loop backwards until we find a difficulty change point,
    // or we find a block which does not have max_bits (is not special).
    while (!is_retarget_height(previous_height))
    {
        previous_block = fetch_block(--previous_height);

        // Test for non-special block.
        if (previous_block.bits != max_work_bits)
            break;
    }

    return previous_block.bits;
}

// TODO: deprecated as unsafe, ignores error code.
chain::header validate_block_impl::fetch_block(size_t height) const
{
    if (height > fork_height_)
    {
        const auto fetch_index = height - fork_height_ - 1;

        // BUGBUG: unsafe suppression of failure condition.
        BITCOIN_ASSERT(fetch_index <= orphan_index_);

        return orphan_chain_[fetch_index]->actual()->header;
    }

    chain::header out;

    // BUGBUG: unsafe suppression of failure condition.
    DEBUG_ONLY(const auto result =) chain_.get_header(out, height);
    BITCOIN_ASSERT(result);
    return out;
}

// ----------------------------------------------------------------------------

// Safe replacement for fetch_block.
bool validate_block_impl::fetch_header(chain::header& header,
    size_t fetch_height) const
{
    if (fetch_height > fork_height_)
    {
        const auto fetch_index = fetch_height - fork_height_ - 1;

        if (fetch_index > orphan_index_)
            return false;

        header = orphan_chain_[fetch_index]->actual()->header;
        return true;
    }

    return chain_.get_header(header, fetch_height);
}

bool validate_block_impl::fetch_transaction(chain::transaction& tx,
    size_t& tx_height, const hash_digest& tx_hash) const
{
    uint64_t out_tx_height;
    const auto result = chain_.get_transaction(tx, out_tx_height, tx_hash);

    if (result && out_tx_height <= fork_height_)
    {
        BITCOIN_ASSERT(out_tx_height <= max_size_t);
        tx_height = static_cast<size_t>(out_tx_height);
        return true;
    }

    return fetch_orphan_transaction(tx, tx_height, tx_hash);
}

bool validate_block_impl::fetch_orphan_transaction(chain::transaction& tx,
    size_t& tx_height, const hash_digest& tx_hash) const
{
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan)
    {
        const auto& orphan_block = orphan_chain_[orphan]->actual();

        for (const auto& orphan_tx: orphan_block->transactions)
        {
            if (orphan_tx.hash() == tx_hash)
            {
                // TRANSACTION COPY
                tx = orphan_tx;
                tx_height = fork_height_ + orphan + 1;
                return true;
            }
        }
    }

    return false;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point& outpoint) const
{
    uint64_t tx_height;
    hash_digest tx_hash;
    return
        chain_.get_outpoint_transaction(tx_hash, outpoint) &&
        chain_.get_transaction_height(tx_height, tx_hash) &&
        tx_height <= fork_height_;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point& previous_output, size_t position,
    uint32_t input_index) const
{
    // Search for double spends. This must be done in both chain AND orphan.
    if (is_output_spent(previous_output))
        return true;

    if (is_orphan_spent(previous_output, position, input_index))
        return true;

    return false;
}

bool validate_block_impl::is_orphan_spent(
    const chain::output_point& previous_output,
    size_t skip_position, uint32_t skip_input_index) const
{
    // This gets costly as the size of the orphan pool increases.
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan)
    {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        const auto& transactions = orphan_block->transactions;
        BITCOIN_ASSERT(!transactions.empty());
        BITCOIN_ASSERT(transactions.front().is_coinbase());

        for (size_t position = 0; position < transactions.size(); ++position)
        {
            // TODO: too visually deep, move this section to subfunction.
            const auto& orphan_tx = transactions[position];
            const auto inputs = orphan_tx.inputs.size();
            BITCOIN_ASSERT(inputs <= max_uint32);

            for (uint32_t input_index = 0; input_index < inputs; ++input_index)
            {
                const auto& orphan_input = orphan_tx.inputs[input_index];

                if (orphan == orphan_index_ && position == skip_position &&
                    input_index == skip_input_index)
                    continue;

                if (orphan_input.previous_output == previous_output)
                    return true;
            }
        }
    }

    return false;
}

} // namespace blockchain
} // namespace libbitcoin
