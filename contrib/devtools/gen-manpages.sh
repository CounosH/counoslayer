#!/usr/bin/env bash
# Copyright (c) 2016-2019 The CounosH Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C
TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
BUILDDIR=${BUILDDIR:-$TOPDIR}

BINDIR=${BINDIR:-$BUILDDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

COUNOSHD=${COUNOSHD:-$BINDIR/counoshd}
COUNOSHCLI=${COUNOSHCLI:-$BINDIR/counosh-cli}
COUNOSHTX=${COUNOSHTX:-$BINDIR/counosh-tx}
WALLET_TOOL=${WALLET_TOOL:-$BINDIR/counosh-wallet}
COUNOSHQT=${COUNOSHQT:-$BINDIR/qt/counosh-qt}

[ ! -x $COUNOSHD ] && echo "$COUNOSHD not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
read -r -a CCHVER <<< "$($COUNOSHCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }')"

# Create a footer file with copyright content.
# This gets autodetected fine for counoshd if --version-string is not set,
# but has different outcomes for counosh-qt and counosh-cli.
echo "[COPYRIGHT]" > footer.h2m
$COUNOSHD --version | sed -n '1!p' >> footer.h2m

for cmd in $COUNOSHD $COUNOSHCLI $COUNOSHTX $WALLET_TOOL $COUNOSHQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${CCHVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${CCHVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
