###############################################################################
#  Copyright (c) 2014-2015 libbitcoin-blockchain developers (see COPYING).
#
#         GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
#
###############################################################################

# Automake settings.
#==============================================================================
# Look for macros in the m4 subdirectory.
#------------------------------------------------------------------------------
ACLOCAL_AMFLAGS = -I m4


# Distribute, make and install products.
#==============================================================================
# files => ${pkgconfigdir}
#------------------------------------------------------------------------------
pkgconfig_DATA = \
    libbitcoin-blockchain.pc

# files => ${docdir}
#------------------------------------------------------------------------------
doc_DATA = \
    AUTHORS \
    COPYING \
    ChangeLog \
    INSTALL \
    NEWS \
    README

# src/libbitcoin-blockchain.la => ${libdir}
#------------------------------------------------------------------------------
lib_LTLIBRARIES = src/libbitcoin-blockchain.la
src_libbitcoin_blockchain_la_CPPFLAGS = -I${srcdir}/include ${bitcoin_database_BUILD_CPPFLAGS} ${bitcoin_consensus_BUILD_CPPFLAGS}
src_libbitcoin_blockchain_la_LIBADD = ${bitcoin_database_LIBS} ${bitcoin_consensus_LIBS}
src_libbitcoin_blockchain_la_SOURCES = \
    src/settings.cpp \
    src/interface/block_chain.cpp \
    src/organizers/block_organizer.cpp \
    src/organizers/header_organizer.cpp \
    src/organizers/transaction_organizer.cpp \
    src/pools/anchor_converter.cpp \
    src/pools/child_closure_calculator.cpp \
    src/pools/conflicting_spend_remover.cpp \
    src/pools/header_branch.cpp \
    src/pools/header_entry.cpp \
    src/pools/header_pool.cpp \
    src/pools/parent_closure_calculator.cpp \
    src/pools/priority_calculator.cpp \
    src/pools/stack_evaluator.cpp \
    src/pools/transaction_entry.cpp \
    src/pools/transaction_order_calculator.cpp \
    src/pools/transaction_pool.cpp \
    src/pools/transaction_pool_state.cpp \
    src/populate/populate_base.cpp \
    src/populate/populate_block.cpp \
    src/populate/populate_chain_state.cpp \
    src/populate/populate_header.cpp \
    src/populate/populate_transaction.cpp \
    src/validate/validate_block.cpp \
    src/validate/validate_header.cpp \
    src/validate/validate_input.cpp \
    src/validate/validate_transaction.cpp

# local: test/libbitcoin-blockchain-test
#------------------------------------------------------------------------------
if WITH_TESTS

TESTS = libbitcoin-blockchain-test_runner.sh

check_PROGRAMS = test/libbitcoin-blockchain-test
test_libbitcoin_blockchain_test_CPPFLAGS = -I${srcdir}/include ${bitcoin_database_BUILD_CPPFLAGS} ${bitcoin_consensus_BUILD_CPPFLAGS}
test_libbitcoin_blockchain_test_LDADD = src/libbitcoin-blockchain.la ${boost_unit_test_framework_LIBS} ${bitcoin_database_LIBS} ${bitcoin_consensus_LIBS}
test_libbitcoin_blockchain_test_SOURCES = \
    test/fast_chain.cpp \
    test/header_branch.cpp \
    test/header_entry.cpp \
    test/header_pool.cpp \
    test/main.cpp \
    test/safe_chain.cpp \
    test/transaction_entry.cpp \
    test/transaction_pool.cpp \
    test/utility.cpp \
    test/utility.hpp \
    test/validate_block.cpp \
    test/validate_transaction.cpp \
    test/pools/anchor_converter.cpp \
    test/pools/child_closure_calculator.cpp \
    test/pools/conflicting_spend_remover.cpp \
    test/pools/parent_closure_calculator.cpp \
    test/pools/priority_calculator.cpp \
    test/pools/transaction_order_calculator.cpp \
    test/pools/utilities.cpp \
    test/pools/utilities.hpp

endif WITH_TESTS

# local: tools/initchain/initchain
#------------------------------------------------------------------------------
if WITH_TOOLS

noinst_PROGRAMS = tools/initchain/initchain
tools_initchain_initchain_CPPFLAGS = -I${srcdir}/include ${bitcoin_database_BUILD_CPPFLAGS} ${bitcoin_consensus_BUILD_CPPFLAGS}
tools_initchain_initchain_LDADD = src/libbitcoin-blockchain.la ${bitcoin_database_LIBS} ${bitcoin_consensus_LIBS}
tools_initchain_initchain_SOURCES = \
    tools/initchain/initchain.cpp

endif WITH_TOOLS

# files => ${includedir}/bitcoin
#------------------------------------------------------------------------------
include_bitcoindir = ${includedir}/bitcoin
include_bitcoin_HEADERS = \
    include/bitcoin/blockchain.hpp

include_bitcoin_blockchaindir = ${includedir}/bitcoin/blockchain
include_bitcoin_blockchain_HEADERS = \
    include/bitcoin/blockchain/define.hpp \
    include/bitcoin/blockchain/settings.hpp \
    include/bitcoin/blockchain/version.hpp

include_bitcoin_blockchain_interfacedir = ${includedir}/bitcoin/blockchain/interface
include_bitcoin_blockchain_interface_HEADERS = \
    include/bitcoin/blockchain/interface/block_chain.hpp \
    include/bitcoin/blockchain/interface/fast_chain.hpp \
    include/bitcoin/blockchain/interface/safe_chain.hpp

include_bitcoin_blockchain_organizersdir = ${includedir}/bitcoin/blockchain/organizers
include_bitcoin_blockchain_organizers_HEADERS = \
    include/bitcoin/blockchain/organizers/block_organizer.hpp \
    include/bitcoin/blockchain/organizers/header_organizer.hpp \
    include/bitcoin/blockchain/organizers/transaction_organizer.hpp

include_bitcoin_blockchain_poolsdir = ${includedir}/bitcoin/blockchain/pools
include_bitcoin_blockchain_pools_HEADERS = \
    include/bitcoin/blockchain/pools/anchor_converter.hpp \
    include/bitcoin/blockchain/pools/child_closure_calculator.hpp \
    include/bitcoin/blockchain/pools/conflicting_spend_remover.hpp \
    include/bitcoin/blockchain/pools/header_branch.hpp \
    include/bitcoin/blockchain/pools/header_entry.hpp \
    include/bitcoin/blockchain/pools/header_pool.hpp \
    include/bitcoin/blockchain/pools/parent_closure_calculator.hpp \
    include/bitcoin/blockchain/pools/priority_calculator.hpp \
    include/bitcoin/blockchain/pools/stack_evaluator.hpp \
    include/bitcoin/blockchain/pools/transaction_entry.hpp \
    include/bitcoin/blockchain/pools/transaction_order_calculator.hpp \
    include/bitcoin/blockchain/pools/transaction_pool.hpp \
    include/bitcoin/blockchain/pools/transaction_pool_state.hpp

include_bitcoin_blockchain_populatedir = ${includedir}/bitcoin/blockchain/populate
include_bitcoin_blockchain_populate_HEADERS = \
    include/bitcoin/blockchain/populate/populate_base.hpp \
    include/bitcoin/blockchain/populate/populate_block.hpp \
    include/bitcoin/blockchain/populate/populate_chain_state.hpp \
    include/bitcoin/blockchain/populate/populate_header.hpp \
    include/bitcoin/blockchain/populate/populate_transaction.hpp

include_bitcoin_blockchain_validatedir = ${includedir}/bitcoin/blockchain/validate
include_bitcoin_blockchain_validate_HEADERS = \
    include/bitcoin/blockchain/validate/validate_block.hpp \
    include/bitcoin/blockchain/validate/validate_header.hpp \
    include/bitcoin/blockchain/validate/validate_input.hpp \
    include/bitcoin/blockchain/validate/validate_transaction.hpp


# Custom make targets.
#==============================================================================
# make target: tools
#------------------------------------------------------------------------------
target_tools = \
    tools/initchain/initchain

tools: ${target_tools}

