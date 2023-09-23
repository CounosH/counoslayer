#!/usr/bin/env bash

export LC_ALL=C

# Get BUILDDIR
CURDIR=$(cd $(dirname "$0") || exit; pwd)
# shellcheck source=/dev/null
. "${CURDIR}/../config.ini"

if [ -z "$BUILDDIR" ]; then
  BUILDDIR="$CURDIR";
  TESTDIR="$CURDIR/test/tmp/counoscore-rpc-tests";
else
  TESTDIR="$BUILDDIR/test/tmp/counoscore-rpc-tests"
fi

COUNOSCORED="$BUILDDIR/src/counoscored$EXEEXT"
COUNOSCORECLI="$BUILDDIR/src/counoscore-cli$EXEEXT"
DATADIR="$TESTDIR/.bitcoin"

# Start clean
rm -rf "$BUILDDIR/test/tmp"

git clone https://github.com/CounosLayer/CounosJ.git $TESTDIR
mkdir -p "$DATADIR/regtest"
touch "$DATADIR/regtest/counoscore.log"
cd $TESTDIR || exit
echo "Counos Core RPC test dir: "$TESTDIR
echo "Last CounosJ commit: "$(git log -n 1 --format="%H Author: %cn <%ce>")
if [ "$1" = "true" ]; then
    echo "Debug logging level: maximum"
    $COUNOSCORED -datadir="$DATADIR" -regtest -txindex -server -daemon -rpcuser=bitcoinrpc -rpcpassword=pass -debug=1 -counosdebug=all -counosalertallowsender=any -counosactivationallowsender=any -paytxfee=0.0001 -minrelaytxfee=0.00001 -limitancestorcount=750 -limitdescendantcount=750 -rpcserialversion=0 -discover=0 -listen=0 -acceptnonstdtxn=1 &
else
    echo "Debug logging level: minimum"
    $COUNOSCORED -datadir="$DATADIR" -regtest -txindex -server -daemon -rpcuser=bitcoinrpc -rpcpassword=pass -debug=0 -counosdebug=none -counosalertallowsender=any -counosactivationallowsender=any -paytxfee=0.0001 -minrelaytxfee=0.00001 -limitancestorcount=750 -limitdescendantcount=750 -rpcserialversion=0 -discover=0 -listen=0 -acceptnonstdtxn=1 &
fi
$COUNOSCORECLI -datadir="$DATADIR" -regtest -rpcuser=bitcoinrpc -rpcpassword=pass -rpcwait getblockchaininfo
$COUNOSCORECLI -datadir="$DATADIR" -regtest -rpcuser=bitcoinrpc -rpcpassword=pass -rpcwait counos_getinfo
./gradlew --console plain :counosj-rpc:regTest
STATUS=$?
$COUNOSCORECLI -datadir="$DATADIR" -regtest -rpcuser=bitcoinrpc -rpcpassword=pass -rpcwait stop

# If $STATUS is not 0, the test failed.
if [ $STATUS -ne 0 ]; then tail -100 $DATADIR/regtest/counoscore.log; fi


exit $STATUS
