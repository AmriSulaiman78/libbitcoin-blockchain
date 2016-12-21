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
#ifndef LIBBITCOIN_BLOCKCHAIN_BLOCK_ENTRY_HPP
#define LIBBITCOIN_BLOCKCHAIN_BLOCK_ENTRY_HPP

#include <vector>
#include <boost/functional/hash_fwd.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is not thread safe.
class BCB_API block_entry
{
public:
    typedef std::vector<hash_digest> hashes;

    /// Construct an entry for the pool.
    /// Never store an invalid block in the pool.
    block_entry(block_const_ptr block);

    /// Use this construction only as a search key.
    block_entry(const hash_digest& hash);

    /// The block that the entry contains.
    block_const_ptr block() const;

    /// The hash table entry identity.
    const hash_digest& hash() const;

    /// The hash table entry's parent (preceding block) hash.
    const hash_digest& parent() const;

    /// The hash table entry's child (succeeding block) hashes.
    const hashes& children() const;

    /// Add block to the list of children of this block.
    void add_child(block_const_ptr child) const;

    /// Add block to the list of children of this block.
    void remove_child(block_const_ptr child) const;

    /// Operators.
    bool operator==(const block_entry& other) const;

private:
    const hash_digest hash_;
    const block_const_ptr block_;

    // TODO: could save some bytes here by holding the pointer in place of the
    // hash. This would allow navigation to the hash saving 24 bytes per child.
    // Children do not pertain to entry hash, so can be mutable.
    mutable hashes children_;
};

} // namespace blockchain
} // namespace libbitcoin

// Standard (boost) hash.
//-----------------------------------------------------------------------------

namespace boost
{

// Extend boost namespace with our block_const_ptr hash function.
template <>
struct hash<bc::blockchain::block_entry>
{
    size_t operator()(const bc::blockchain::block_entry& entry) const
    {
        return boost::hash<bc::hash_digest>()(entry.hash());
    }
};

} // namespace boost

#endif
