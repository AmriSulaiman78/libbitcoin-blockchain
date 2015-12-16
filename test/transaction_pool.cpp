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
#include <memory>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <bitcoin/blockchain.hpp>

using namespace bc;
using namespace bc::chain;

BOOST_AUTO_TEST_SUITE(transaction_pool_tests)

class threadpool_fixture
  : public threadpool
{
};

class blockchain_fixture
  : public blockchain
{
public:
    virtual bool start()
    {
        return false;
    }

    virtual bool stop()
    {
        return false;
    }

    virtual void store(const block_type& block,
        store_block_handler handle_store)
    {
    }

    virtual void import(const block_type& import_block,
        import_block_handler handle_import)
    {
    }

    virtual void fetch_block_header(uint64_t height,
        fetch_handler_block_header handle_fetch)
    {
    }

    virtual void fetch_block_header(const hash_digest& hash,
        fetch_handler_block_header handle_fetch)
    {
    }

    virtual void fetch_missing_block_hashes(const hash_list& hashes,
        fetch_handler_missing_block_hashes handle_fetch)
    {
    }

    virtual void fetch_block_transaction_hashes(const hash_digest& hash,
        fetch_handler_block_transaction_hashes handle_fetch)
    {
    }

    virtual void fetch_block_height(const hash_digest& hash,
        fetch_handler_block_height handle_fetch)
    {
    }

    virtual void fetch_last_height(fetch_handler_last_height handle_fetch)
    {
    }

    virtual void fetch_transaction(const hash_digest& hash,
        fetch_handler_transaction handle_fetch)
    {
    }

    virtual void fetch_transaction_index(const hash_digest& hash,
        fetch_handler_transaction_index handle_fetch)
    {
    }

    virtual void fetch_spend(const output_point& outpoint,
        fetch_handler_spend handle_fetch)
    {
    }

    virtual void fetch_history(const payment_address& address,
        fetch_handler_history handle_fetch, const uint64_t limit=0,
        const uint64_t from_height=0)
    {
    }

    virtual void fetch_stealth(const binary_type& prefix,
        fetch_handler_stealth handle_fetch, uint64_t from_height=0)
    {
    }

    virtual void subscribe_reorganize(reorganize_handler handle_reorganize)
    {
    }
};

class transaction_pool_fixture
  : public transaction_pool
{
public:
    transaction_pool_fixture(threadpool& pool, blockchain& chain,
        size_t capacity)
      : transaction_pool(pool, chain, capacity)
    {
    }

    // Set capcity based on test buffer size.
    transaction_pool_fixture(threadpool& pool, blockchain& chain,
        const pool_buffer& txs)
      : transaction_pool(pool, chain, txs.capacity())
    {
        // Start by default, fill with our test buffer data.
        stopped_ = false;
        for (const auto& entry: txs)
            buffer_.push_back(entry);
    }

    // Test accesors.

    void add(const transaction_type& tx, confirm_handler handler)
    {
        transaction_pool::add(tx, handler);
    }

    void delete_all(const std::error_code& ec)
    {
        transaction_pool::delete_all(ec);
    }

    void delete_package(const std::error_code& ec)
    {
        transaction_pool::delete_package(ec);
    }

    void delete_package(const hash_digest& tx_hash, const code& ec)
    {
        transaction_pool::delete_package(tx_hash, ec);
    }

    void delete_package(const transaction_type& tx, const code& ec)
    {
        transaction_pool::delete_package(tx, ec);
    }

    void delete_single(const hash_digest& tx_hash, const code& ec)
    {
        transaction_pool::delete_single(tx_hash, ec);
    }

    void delete_single(const transaction_type& tx, const code& ec)
    {
        transaction_pool::delete_single(tx, ec);
    }

    void delete_dependencies(const hash_digest& tx_hash, const code& ec)
    {
        transaction_pool::delete_dependencies(tx_hash, ec);
    }

    void delete_dependencies(const output_point& point, const code& ec)
    {
        transaction_pool::delete_dependencies(point, ec);
    }

    void delete_dependencies(input_comparison is_dependency, const code& ec)
    {
        transaction_pool::delete_dependencies(is_dependency, ec);
    }

    void delete_superseded(const blockchain::block_list& blocks)
    {
        transaction_pool::delete_superseded(blocks);
    }

    void delete_confirmed_in_blocks(const blockchain::block_list& blocks)
    {
        transaction_pool::delete_confirmed_in_blocks(blocks);
    }

    void delete_spent_in_blocks(const blockchain::block_list& blocks)
    {
        transaction_pool::delete_spent_in_blocks(blocks);
    }

    pool_buffer& transactions()
    {
        return buffer_;
    }

    void stopped(bool stop)
    {
        stopped_ = stop;
    }
};

#define DECLARE_TRANSACTION_POOL(pool, txs) \
    threadpool_fixture memory_pool; \
    blockchain_fixture block_chain; \
    transaction_pool_fixture pool(memory_pool, block_chain, txs)

#define DECLARE_TRANSACTION(number, code) \
    transaction_type tx##number; \
    tx##number.locktime = number; \
    const size_t tx##number##_id = number; \
    auto hash##number = hash_transaction(tx##number); \
    std::error_code result##number(error::unknown); \
    const auto handle_confirm##number = [&result##number](const std::error_code& ec) \
    { \
        result##number = ec; \
        BOOST_CHECK_EQUAL(ec.value(), code); \
    }; \
    transaction_entry_info entry##number{ hash##number, tx##number, handle_confirm##number }

#define REQUIRE_CALLBACK(number, code) \
    BOOST_REQUIRE_EQUAL(result##number, code)

#define TX_ID_AT_POSITION(pool, position) \
    pool.transactions()[position].tx.locktime

#define ADD_INPUT_TO_TX_NUMBER(number, prevout_hash, prevout_index) \
    tx##number.inputs.push_back({ { prevout_hash, prevout_index }, {}, 0 }); \
    hash##number = hash_transaction(tx##number); \
    entry##number = { hash##number, tx##number, handle_confirm##number }

BOOST_AUTO_TEST_SUITE(transaction_pool__construct)

BOOST_AUTO_TEST_CASE(transaction_pool__construct1__always__does_not_throw)
{
    threadpool_fixture pool;
    blockchain_fixture chain;
    BOOST_REQUIRE_NO_THROW(transaction_pool_fixture(pool, chain, 0));
}

BOOST_AUTO_TEST_CASE(transaction_pool__construct2__zero__zero)
{
    pool_buffer transactions;
    DECLARE_TRANSACTION_POOL(mempool, transactions);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__construct2__one__one_destructor_callback)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(2);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__add)

BOOST_AUTO_TEST_CASE(transaction_pool__add__zero_capacity__empty)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer transactions(0);
    DECLARE_TRANSACTION_POOL(mempool, transactions);
    mempool.add(tx0, handle_confirm0);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__add__empty__one)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer transactions(2);
    DECLARE_TRANSACTION_POOL(mempool, transactions);
    mempool.add(tx0, handle_confirm0);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__add__overflow_two__oldest_two_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::pool_filled);
    DECLARE_TRANSACTION(1, error::pool_filled);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer transactions(1);
    DECLARE_TRANSACTION_POOL(mempool, transactions);
    mempool.add(tx0, handle_confirm0);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    mempool.add(tx1, handle_confirm1);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx1_id);
    mempool.add(tx2, handle_confirm2);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx2_id);
    REQUIRE_CALLBACK(0, error::pool_filled);
    REQUIRE_CALLBACK(1, error::pool_filled);
}

BOOST_AUTO_TEST_CASE(transaction_pool__add__overflow_with_dependencies__removes_oldest_and_dependencies)
{
    DECLARE_TRANSACTION(0, error::pool_filled);
    DECLARE_TRANSACTION(1, error::pool_filled);
    DECLARE_TRANSACTION(2, error::pool_filled);
    DECLARE_TRANSACTION(3, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(3);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.add(tx3, handle_confirm3);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx3_id);
    REQUIRE_CALLBACK(0, error::pool_filled);
    REQUIRE_CALLBACK(1, error::pool_filled);
    REQUIRE_CALLBACK(2, error::pool_filled);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_all)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_all__empty__empty)
{
    pool_buffer transactions(2);
    DECLARE_TRANSACTION_POOL(mempool, transactions);
    mempool.delete_all(error::unknown);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_all__two__empty_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::network_unreachable);
    DECLARE_TRANSACTION(1, error::network_unreachable);
    pool_buffer buffer(2);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    mempool.delete_all(error::network_unreachable);
    BOOST_REQUIRE(mempool.transactions().empty());
    REQUIRE_CALLBACK(0, error::network_unreachable);
    REQUIRE_CALLBACK(1, error::network_unreachable);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_all__stopped_two__empty_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::address_blocked);
    DECLARE_TRANSACTION(1, error::address_blocked);
    pool_buffer buffer(2);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_all(error::address_blocked);
    BOOST_REQUIRE(mempool.transactions().empty());
    REQUIRE_CALLBACK(0, error::address_blocked);
    REQUIRE_CALLBACK(1, error::address_blocked);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_package)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package1__empty__empty)
{
    pool_buffer buffer(5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package1__three__oldest_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package1__stopped__unchanged_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_package(error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package1__dependencies__deletes_self_and_dependencies)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
    REQUIRE_CALLBACK(2, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package2__empty__empty)
{
    pool_buffer buffer(5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(null_hash, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package2__three__match_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(hash1, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package2__no_match__no_change_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(null_hash, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package2__stopped__unchanged_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_package(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package2__dependencies__deletes_self_and_dependencies)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
    REQUIRE_CALLBACK(2, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package3__empty__empty)
{
    DECLARE_TRANSACTION(0, 42);
    pool_buffer buffer(5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package3__three__match_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(tx1, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package3__no_match__no_change_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    DECLARE_TRANSACTION(3, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(tx3, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package3__stopped__unchanged_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_package(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_package3__dependencies__deletes_self_and_dependencies)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_package(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
    REQUIRE_CALLBACK(2, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_SUITE_END()BOOST_AUTO_TEST_SUITE(transaction_pool__delete_single)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single1__empty__empty)
{
    pool_buffer buffer(5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(null_hash, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single1__three__match_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);

    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(hash1, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single1__no_match__no_change_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(null_hash, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single1__stopped__unchanged_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_single(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single1__dependencies__deletes_self_only)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single2__empty__empty)
{
    DECLARE_TRANSACTION(0, 42);
    pool_buffer buffer(5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single2__three__match_removed_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(tx1, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single2__no_match__no_change_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    DECLARE_TRANSACTION(3, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(tx3, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single2__stopped__unchanged_expected_callbacks)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_single(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_single2__dependencies__deletes_self_only)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_single(tx0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_dependencies)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__empty__empty)
{
    pool_buffer buffer(0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(output_point{ null_hash, 0 }, error::unknown);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__forward_full_chain__head_remains)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(output_point{ hash0, 42 }, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
    REQUIRE_CALLBACK(2, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__reverse_full_chain__head_remains)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash2, 24);
    ADD_INPUT_TO_TX_NUMBER(0, hash1, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(output_point{ hash2, 24 }, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx2_id);
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__partial_chain__expected)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash2, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(output_point{ hash2, 24 }, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__multiple_chlidren__removed_by_index)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 24);
    ADD_INPUT_TO_TX_NUMBER(2, hash0, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(output_point{ hash0, 24 }, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies1__stopped_full_chain__unchanged)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_dependencies(output_point{ hash0, 42 }, error::unknown);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__empty__empty)
{
    pool_buffer buffer(0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(null_hash, error::unknown);
    BOOST_REQUIRE(mempool.transactions().empty());
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__forward_full_chain__head_remains)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
    REQUIRE_CALLBACK(2, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__reverse_full_chain__head_remains)
{
    DECLARE_TRANSACTION(0, error::futuristic_timestamp);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash2, 24);
    ADD_INPUT_TO_TX_NUMBER(0, hash1, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(hash2, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx2_id);
    REQUIRE_CALLBACK(0, error::futuristic_timestamp);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__partial_chain__expected)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash2, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(hash2, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__multiple_chlidren__all_removed)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::futuristic_timestamp);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 24);
    ADD_INPUT_TO_TX_NUMBER(2, hash0, 42);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(hash0, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies2__stopped_full_chain__unchanged)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.stopped(true);
    mempool.delete_dependencies(hash0, error::unknown);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 3u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx1_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx2_id);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_dependencies3__custom_comparison_partial_chain__expected)
{
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::futuristic_timestamp);
    DECLARE_TRANSACTION(2, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(2, hash0, 42);
    ADD_INPUT_TO_TX_NUMBER(1, hash2, 24);
    pool_buffer buffer(5);
    buffer.push_back(entry0);
    buffer.push_back(entry1);
    buffer.push_back(entry2);

    // ONLY match hash1 (so entry2 will not be deleted).
    const auto comparison = [&hash1](const transaction_input_type& input)
    {
        return input.previous_output.hash == hash1;
    };

    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_dependencies(hash2, error::futuristic_timestamp);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx0_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx2_id);
    REQUIRE_CALLBACK(1, error::futuristic_timestamp);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_confirmed_in_blocks)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_confirmed_in_blocks__empty_block__expected)
{
    blockchain::block_list blocks;
    block_type block1;
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(1);
    DECLARE_TRANSACTION(0, error::service_stopped);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_confirmed_in_blocks(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_confirmed_in_blocks__one_block_no_dependencies__expected)
{
    blockchain::block_list blocks;
    block_type block1;
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::success);
    DECLARE_TRANSACTION(2, error::success);
    DECLARE_TRANSACTION(3, error::service_stopped);
    block1.transactions.push_back(tx0);
    block1.transactions.push_back(tx1);
    block1.transactions.push_back(tx2);
    block1.transactions.push_back(tx3);
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(5);
    DECLARE_TRANSACTION(4, error::service_stopped);
    buffer.push_back(entry2);
    buffer.push_back(entry4);
    buffer.push_back(entry1);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_confirmed_in_blocks(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx4_id);
    REQUIRE_CALLBACK(1, error::success);
    REQUIRE_CALLBACK(2, error::success);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_confirmed_in_blocks__two_blocks_dependencies__expected)
{
    blockchain::block_list blocks;
    block_type block1;
    block_type block2;
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::success);
    DECLARE_TRANSACTION(2, error::success);
    block1.transactions.push_back(tx0);
    block2.transactions.push_back(tx1);
    block2.transactions.push_back(tx2);
    blocks.push_back(std::make_shared<block_type>(block1));
    blocks.push_back(std::make_shared<block_type>(block2));
    pool_buffer buffer(8);
    DECLARE_TRANSACTION(3, error::service_stopped);
    DECLARE_TRANSACTION(4, error::service_stopped);
    DECLARE_TRANSACTION(5, error::service_stopped);
    DECLARE_TRANSACTION(6, error::service_stopped);
    DECLARE_TRANSACTION(7, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(3, hash1, 42);
    ADD_INPUT_TO_TX_NUMBER(4, hash3, 43);
    ADD_INPUT_TO_TX_NUMBER(5, hash4, 44);
    ADD_INPUT_TO_TX_NUMBER(6, hash4, 45);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    buffer.push_back(entry3);
    buffer.push_back(entry4);
    buffer.push_back(entry5);
    buffer.push_back(entry6);
    buffer.push_back(entry7);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_confirmed_in_blocks(blocks);
    BOOST_CHECK_EQUAL(mempool.transactions().size(), 5u);
    BOOST_CHECK_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx3_id);
    BOOST_CHECK_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx4_id);
    BOOST_CHECK_EQUAL(TX_ID_AT_POSITION(mempool, 2), tx5_id);
    BOOST_CHECK_EQUAL(TX_ID_AT_POSITION(mempool, 3), tx6_id);
    BOOST_CHECK_EQUAL(TX_ID_AT_POSITION(mempool, 4), tx7_id);
    REQUIRE_CALLBACK(1, error::success);
    REQUIRE_CALLBACK(2, error::success);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_spent_in_blocks)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_spent_in_blocks__empty_block__expected)
{
    blockchain::block_list blocks;
    block_type block1;
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(1);
    DECLARE_TRANSACTION(0, error::service_stopped);
    buffer.push_back(entry0);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_spent_in_blocks(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_spent_in_blocks__two_blocks_no_duplicates_or_dependencies__expected)
{
    blockchain::block_list blocks;
    block_type block1;
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    DECLARE_TRANSACTION(3, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(0, null_hash, 42);
    ADD_INPUT_TO_TX_NUMBER(1, null_hash, 43);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 44);
    ADD_INPUT_TO_TX_NUMBER(2, null_hash, 45);
    ADD_INPUT_TO_TX_NUMBER(2, hash0, 46);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 47);
    ADD_INPUT_TO_TX_NUMBER(3, null_hash, 48);
    ADD_INPUT_TO_TX_NUMBER(3, hash0, 49);
    ADD_INPUT_TO_TX_NUMBER(3, hash1, 59);
    ADD_INPUT_TO_TX_NUMBER(3, hash2, 51);
    block1.transactions.push_back(tx0);
    block1.transactions.push_back(tx1);
    block1.transactions.push_back(tx2);
    block1.transactions.push_back(tx3);
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(5);
    DECLARE_TRANSACTION(4, error::double_spend);
    DECLARE_TRANSACTION(5, error::service_stopped);
    DECLARE_TRANSACTION(6, error::service_stopped);
    DECLARE_TRANSACTION(7, error::double_spend);
    ADD_INPUT_TO_TX_NUMBER(4, hash0, 46);
    ADD_INPUT_TO_TX_NUMBER(5, hash1, 99);
    ADD_INPUT_TO_TX_NUMBER(6, hash2, 99);
    ADD_INPUT_TO_TX_NUMBER(7, hash2, 51);
    buffer.push_back(entry4);
    buffer.push_back(entry5);
    buffer.push_back(entry6);
    buffer.push_back(entry7);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_spent_in_blocks(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx5_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx6_id);
    REQUIRE_CALLBACK(4, error::double_spend);
    REQUIRE_CALLBACK(7, error::double_spend);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(transaction_pool__delete_superseded)

BOOST_AUTO_TEST_CASE(transaction_pool__delete_superseded__one_block_duplicates_no_spends__removed_as_succeeded)
{
    blockchain::block_list blocks;
    block_type block1;
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::success);
    DECLARE_TRANSACTION(2, error::success);
    DECLARE_TRANSACTION(3, error::success);
    block1.transactions.push_back(tx0);
    block1.transactions.push_back(tx1);
    block1.transactions.push_back(tx2);
    block1.transactions.push_back(tx3);
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(5);
    DECLARE_TRANSACTION(4, error::service_stopped);
    DECLARE_TRANSACTION(5, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(4, hash3, 42);
    buffer.push_back(entry1);
    buffer.push_back(entry2);
    buffer.push_back(entry3);
    buffer.push_back(entry4);
    buffer.push_back(entry5);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_superseded(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 2u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx4_id);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 1), tx5_id);
    REQUIRE_CALLBACK(1, error::success);
    REQUIRE_CALLBACK(2, error::success);
    REQUIRE_CALLBACK(3, error::success);
}

BOOST_AUTO_TEST_CASE(transaction_pool__delete_superseded__two_blocks_spends_no_duplicates__removed_as_spent)
{
    blockchain::block_list blocks;
    block_type block1;
    DECLARE_TRANSACTION(0, error::service_stopped);
    DECLARE_TRANSACTION(1, error::service_stopped);
    DECLARE_TRANSACTION(2, error::service_stopped);
    DECLARE_TRANSACTION(3, error::service_stopped);
    ADD_INPUT_TO_TX_NUMBER(0, null_hash, 42);
    ADD_INPUT_TO_TX_NUMBER(1, null_hash, 43);
    ADD_INPUT_TO_TX_NUMBER(1, hash0, 44);
    ADD_INPUT_TO_TX_NUMBER(2, null_hash, 45);
    ADD_INPUT_TO_TX_NUMBER(2, hash0, 46);
    ADD_INPUT_TO_TX_NUMBER(2, hash1, 47);
    ADD_INPUT_TO_TX_NUMBER(3, null_hash, 48);
    ADD_INPUT_TO_TX_NUMBER(3, hash0, 49);
    ADD_INPUT_TO_TX_NUMBER(3, hash1, 59);
    ADD_INPUT_TO_TX_NUMBER(3, hash2, 51);
    block1.transactions.push_back(tx0);
    block1.transactions.push_back(tx1);
    block1.transactions.push_back(tx2);
    block1.transactions.push_back(tx3);
    blocks.push_back(std::make_shared<block_type>(block1));
    pool_buffer buffer(5);
    DECLARE_TRANSACTION(4, error::double_spend);
    DECLARE_TRANSACTION(5, error::double_spend);
    DECLARE_TRANSACTION(6, error::service_stopped);
    DECLARE_TRANSACTION(7, error::double_spend);
    ADD_INPUT_TO_TX_NUMBER(4, hash0, 46);
    ADD_INPUT_TO_TX_NUMBER(5, hash4, 99);
    ADD_INPUT_TO_TX_NUMBER(6, hash2, 99);
    ADD_INPUT_TO_TX_NUMBER(7, hash2, 51);
    buffer.push_back(entry4);
    buffer.push_back(entry5);
    buffer.push_back(entry6);
    buffer.push_back(entry7);
    DECLARE_TRANSACTION_POOL(mempool, buffer);
    mempool.delete_superseded(blocks);
    BOOST_REQUIRE_EQUAL(mempool.transactions().size(), 1u);
    BOOST_REQUIRE_EQUAL(TX_ID_AT_POSITION(mempool, 0), tx6_id);
    REQUIRE_CALLBACK(4, error::double_spend);
    REQUIRE_CALLBACK(5, error::double_spend);
    REQUIRE_CALLBACK(7, error::double_spend);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
