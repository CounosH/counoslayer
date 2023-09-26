#!/usr/bin/env python3
# Copyright (c) 2017-2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test freeze."""

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal

def rollback_chain(node, address):
    # Rolling back the chain to test reversing the last FREEZE tx
    blockcount = node.getblockcount()
    blockhash = node.getblockhash(blockcount)
    node.invalidateblock(blockhash)
    node.clearmempool()
    node.generatetoaddress(1, address)
    new_blockcount = node.getblockcount()
    new_blockhash = node.getblockhash(new_blockcount)

    # Checking the block count is the same as before the rollback...
    assert_equal(blockcount, new_blockcount)

    # Checking the block hash is different from before the rollback...
    if blockhash == new_blockhash:
        raise AssertionError("Block hashes should differ after reorg")

class counosDelegation(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
        self.extra_args = [['-counosactivationallowsender=any']]

    def run_test(self):
        self.log.info("Test delegation of token issuance")

        node = self.nodes[0]

        # Obtaining addresses to work with
        issuer_address = node.getnewaddress()
        delegate_address = node.getnewaddress()
        sink_address = node.getnewaddress()
        unrelated_address = node.getnewaddress()
        coinbase_address = node.getnewaddress()

        # Preparing some mature Bitcoins
        node.generatetoaddress(110, coinbase_address)

        # Funding the addresses with some testnet BTC for fees
        node.sendmany("", {issuer_address: 5, delegate_address: 3, sink_address: 4, unrelated_address: 7})
        node.generatetoaddress(1, coinbase_address)

        # Creating a test (managed) property and granting 1000 tokens to the issuer address
        node.counos_sendissuancemanaged(issuer_address, 1, 1, 0, "TestCat", "TestSubCat", "TestProperty", "TestURL", "TestData")
        node.generatetoaddress(1, coinbase_address)

        # Grant 1000 tokens to sink address
        node.counos_sendgrant(issuer_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)

        # Try to issue new tokens from the delegate address without delegating access to it - expected to fail
        txid = node.counos_sendgrant(delegate_address, sink_address, 3, "55")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Try to name a delegate from an unrelated address - expected to fail
        txid = node.counos_sendadddelegate(unrelated_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Name delegate - pass
        txid = node.counos_sendadddelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], delegate_address)

        # Try to remove the delegate from an unrelated address - expected to fail
        txid = node.counos_sendremovedelegate(unrelated_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Try to remove an unrelated address as issuer - expected to fail
        txid = node.counos_sendremovedelegate(issuer_address, 3, unrelated_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Try to remove an unrelated address as delegate - expected to fail
        txid = node.counos_sendremovedelegate(delegate_address, 3, unrelated_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Remove delegate as delegate - pass
        txid = node.counos_sendremovedelegate(delegate_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], "")

        # Try again to remove delegate as delegate - expected to fail
        txid = node.counos_sendremovedelegate(delegate_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Name delegate again - pass
        txid = node.counos_sendadddelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], delegate_address)

        # Remove delegate as issuer - pass
        txid = node.counos_sendremovedelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], "")

        # Try again to remove delegate as issuer - expected to fail
        txid = node.counos_sendremovedelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Name delegate again - pass
        txid = node.counos_sendadddelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], delegate_address)

        # Try to enable freezing as delegate - expected to fail
        txid = node.counos_sendenablefreezing(delegate_address, 3)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Enable freezing as issuer - pass
        txid = node.counos_sendenablefreezing(issuer_address, 3)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['freezingenabled'], True)

        # Try to disable freezing as delegated - expected to fail
        txid = node.counos_senddisablefreezing(delegate_address, 3)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Freeze tokens as issuer, while delegate is set - expected to fail
        txid = node.counos_sendfreeze(issuer_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Freeze tokens as delegate, while delegate is set - pass
        txid = node.counos_sendfreeze(delegate_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Unfreeze tokens as delegate, while delegate is set - pass
        txid = node.counos_sendunfreeze(delegate_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Remove delegate as delegate - pass
        txid = node.counos_sendremovedelegate(delegate_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Try to freeze tokens as delegate, while delegate is set - expected to fail
        txid = node.counos_sendfreeze(delegate_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Freeze tokens as issuer, while delegate is set - pass
        txid = node.counos_sendfreeze(issuer_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Unfreeze tokens as issuer, while delegate is set - pass
        txid = node.counos_sendunfreeze(issuer_address, sink_address, 3, "1000")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Disable freezing as issuer - pass
        txid = node.counos_senddisablefreezing(issuer_address, 3)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['freezingenabled'], False)

        # Try to grant tokens as holder - expected to fail
        txid = node.counos_sendgrant(sink_address, sink_address, 3, "50")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Try to grant tokens as delegate, while no delegate is set - expected to fail
        txid = node.counos_sendgrant(delegate_address, sink_address, 3, "77")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Grant tokens as issuer, while no delegate is set - pass
        txid = node.counos_sendgrant(issuer_address, sink_address, 3, "33")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Revoke tokens as holder - pass
        txid = node.counos_sendrevoke(sink_address, 3, "1")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Name delegate again - pass
        txid = node.counos_sendadddelegate(issuer_address, 3, delegate_address)
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        info = node.counos_getproperty(3)
        assert_equal(tx['valid'], True)
        assert_equal(info['delegate'], delegate_address)

        # Try to grant tokens as issuer, while delegate is set - expected to fail
        txid = node.counos_sendgrant(issuer_address, sink_address, 3, "45")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], False)

        # Grant tokens as delegate, while delegate is set - pass
        txid = node.counos_sendgrant(delegate_address, sink_address, 3, "53")
        node.generatetoaddress(1, coinbase_address)
        tx = node.counos_gettransaction(txid)
        assert_equal(tx['valid'], True)

        # Check final balance
        sink_balance = node.counos_getbalance(sink_address, 3)
        assert_equal(sink_balance['balance'], "1085")



if __name__ == '__main__':
    counosDelegation().main()