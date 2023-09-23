#!/usr/bin/env python3
# Copyright (c) 2017-2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test Counos fee calculation."""

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal

class CounosFeeCalculation(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True
        self.extra_args = [[], ["-counosdebug=all", "-paytxfee=0.00003"]] # 3,000 Sats per KB

    def run_test(self):
        self.log.info("test fee calculation")

        node0 = self.nodes[0]
        node1 = self.nodes[1]

        # Preparing some mature Bitcoins
        coinbase_address = node0.getnewaddress()
        node0.generatetoaddress(130, coinbase_address)

        # Obtaining a master address to work with
        address = node1.getnewaddress()

        # Send lots of small dust inputs to create a large TX
        for _ in range(3):
            for _ in range(50):
                node0.sendtoaddress(address, 0.00002000)
            node0.generatetoaddress(10, coinbase_address)

        # Test small transaction with single input
        txid = node1.counos_sendissuancefixed(address, 1, 2, 0, "", "", "TST", "", "", "1000")
        node1.generatetoaddress(2, coinbase_address)

        # Checking the transaction was valid...
        result = node1.counos_gettransaction(txid)
        print(result)
        assert_equal(result['valid'], True)

        # Minimum fee is 1KB and 3Sats/byte rate vins to cover 3,000 Sats will be selected.
        assert_equal(len(node1.getrawtransaction(txid, 1)['vin']), 2)

        # Large data
        large_data = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"

        # Create large transaction, will fail if fee requirement not met
        txid = node1.counos_sendissuancecrowdsale(address, 1, 2, 0, large_data, large_data, large_data, large_data, large_data, 1, "1", 32533598549, 0, 0)
        node1.generatetoaddress(1, coinbase_address)

        # Checking the transaction was valid...
        result = node1.counos_gettransaction(txid)
        assert_equal(result['valid'], True)

        # Test DEx calls

        # Create and fund new address
        counos_address = node0.getnewaddress()
        node0.sendtoaddress(counos_address, 20)
        node0.generatetoaddress(1, coinbase_address)

        # Participating in the Exodus crowdsale to obtain some COUNOS
        txid = node0.sendmany("", {"cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3": 10, counos_address: 4})
        node0.generatetoaddress(10, coinbase_address)

        # Create Offer
        txid = node0.counos_senddexsell(counos_address, 1, "1", "1", 10, "0.0001", 1)
        node0.generatetoaddress(1, coinbase_address)

        # Checking the transaction was valid...
        result = node0.counos_gettransaction(txid)
        assert_equal(result['valid'], True)

        # Accept the DEx offer
        txid = node1.counos_senddexaccept(address, counos_address, 1, "1")
        node1.generatetoaddress(1, coinbase_address)

        # Checking the transaction was valid...
        result = node1.counos_gettransaction(txid)
        print(result)
        assert_equal(result['valid'], True)

        # Give buyer bitcoin to pay with, only fee left to pay
        node0.sendtoaddress(address, 1)
        node0.generatetoaddress(1, coinbase_address)

        # Pay for the DEx offer, tests min fee arg to WalletTxBuilder
        txid = node1.counos_senddexpay(address, counos_address, 1, "1")
        node1.generatetoaddress(1, coinbase_address)

        # Checking the transaction was valid...
        result = node1.counos_gettransaction(txid)
        assert_equal(result['purchases'][0]['valid'], True)

if __name__ == '__main__':
    CounosFeeCalculation().main()
