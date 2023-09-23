Counos Core v0.3.1
================

v0.3.1 is a minor release and not consensus critical in terms of the Counos Layer protocol rules. Besides other improvemens, this release provides two new RPCs to create funded transactions, two new RPCs to query token wallet balances, and significant stability and performance gains of Counos Core.

An upgrade is highly recommended, but not required, if you are using Counos Core 0.3.0.

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues

Table of contents
=================

- [Counos Core v0.3.1](#counos-core-v031)
- [Upgrading and downgrading](#upgrading-and-downgrading)
  - [How to upgrade](#how-to-upgrade)
  - [Downgrading](#downgrading)
  - [Compatibility with CounosH Core](#compatibility-with-counosh-core)
- [Notable changes](#notable-changes)
  - [Wiki for guiding new users and developers](#wiki-for-guiding-new-users-and-developers)
  - [Support for offline creation of raw Counos transactions](#support-for-offline-creation-of-raw-counos-transactions)
  - [API to fund Counos Layer transactions from other sources](#api-to-fund-counos-layer-transactions-from-other-sources)
  - [Two new RPCs to get and list all wallet balances](#two-new-rpcs-to-get-and-list-all-wallet-balances)
  - [Information about freeze transactions added to "counos_gettransaction"](#information-about-freeze-transactions-added-to-counos_gettransaction)
  - [Fix behavior of "counos_listtransactions"](#fix-behavior-of-counos_listtransactions)
  - [Add field "issuerbonustokens" to "counos_getcrowdsale"](#add-field-issuerbonustokens-to-counos_getcrowdsale)
  - [Show receiving destination, when sending to self](#show-receiving-destination-when-sending-to-self)
  - [Always show frozen balance in balance RPCs](#always-show-frozen-balance-in-balance-rpcs)
  - [Add "name" field to "counos_getallbalancesforaddress"](#add-name-field-to-counos_getallbalancesforaddress)
  - [Massive performance improvements of counoscored](#massive-performance-improvements-of-counoscored)
  - [Storage of state during initial scanning](#storage-of-state-during-initial-scanning)
  - [Properly restore state, when rolling back blocks](#properly-restore-state-when-rolling-back-blocks)
  - [Avoid deadlock, when parsing transactions](#avoid-deadlock-when-parsing-transactions)
  - [Update checkpoints up to block 520000](#update-checkpoints-up-to-block-520000)
  - [Internal preparations for native Segregated Witness support](#internal-preparations-for-native-segregated-witness-support)
  - [Improved internal database structure](#improved-internal-database-structure)
- [Change log](#change-log)
- [Credits](#credits)

Upgrading and downgrading
=========================

How to upgrade
--------------

If you are running CounosH Core or an older version of Counos Core, shut it down. Wait until it has completely shut down, then copy the new version of `counoscored`, `counoscore-cli` and `counoscore-qt`. On Microsoft Windows the setup routine can be used to automate these steps.

During the first startup historical Counos transactions are reprocessed and Counos Core will not be usable for approximately 15 minutes up to two hours. The progress of the initial scan is reported on the console, the GUI and written to the `debug.log`. The scan may be interrupted, but can not be resumed, and then needs to start from the beginning.

Downgrading
-----------

Downgrading to an Counos Core version prior to 0.3.0 is generally not supported as older versions will not provide accurate information due to the changes in consensus rules.

Compatibility with CounosH Core
-------------------------------

Counos Core is based on CounosH Core 0.13.2 and can be used as replacement for CounosH Core. Switching between Counos Core and CounosH Core may be supported.

Upgrading to a higher CounosH Core version is generally supported, but when downgrading from CounosH Core 0.15, Counos Core needs to be started with `-reindex-chainstate` flag, to rebuild the chainstate data structures in a compatible format.

Downgrading to a CounosH Core version prior to 0.12 may not be supported due to the obfuscation of the blockchain database. In this case the database also needs to be rebuilt by starting Counos Core with `-reindex-chainstate` flag.

Downgrading to a CounosH Core version prior to 0.10 is not supported due to the new headers-first synchronization.

Notable changes
===============

Wiki for guiding new users and developers
-----------------------------------------

To help and guide new users and developers, a wiki was created. The wiki includes pointers to other resources such as the JSON-RPC documentation, an overview of startup and configuration options or build instructions. It also includes guides and answers to frequently asked questions:

https://github.com/CounosLayer/counoscore/wiki

Support for offline creation of raw Counos transactions
-----------------------------------------------------

The raw transaction interface can be used to manually craft Counos Layer transactions. With this release, it is no longer necessary to use a fully synchronized client and an offline client can be used.

An overview of the JSON-RPC commands can be found [here](https://github.com/CounosLayer/counoscore/blob/master/src/counoscore/doc/rpc-api.md#raw-transactions) and a guide for the manual creation of a Simple Send transaction is [available](https://github.com/CounosLayer/counoscore/wiki/Use-the-raw-transaction-API-to-create-a-Simple-Send-transaction) in the new wiki.

API to fund Counos Layer transactions from other sources
------------------------------------------------------

This release adds two new RPCs "counos_funded_send" and "counos_funded_sendall", which allow the creation of Counos Layer transactions, which are funded by a different source, other than the original sender.

This can be used to pay for transaction fees, when the sender only has a tiny fraction of coins available, but not enough to cover whole fee of a transaction:

---

#### counos_funded_send

Creates and sends a funded simple send transaction.

All counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `amount`            | string  | required | the amount to send                                                                           |
| `feeaddress`        | string  | required | the address that is used to pay for fees, if needed                                          |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_funded_send" "1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH" \
    "15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH" 1 "100.0" \
    "15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1"
```

---

#### counos_funded_sendall

Creates and sends a transaction that transfers all available tokens in the given ecosystem to the recipient.

All counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `ecosystem`         | number  | required | the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)             |
| `feeaddress`        | string  | required | the address that is used to pay for fees, if needed                                          |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_funded_sendall" "1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH" \
    "15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH" 1 "15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1"
```

---

Two new RPCs to get and list all wallet balances
------------------------------------------------

Counos Core v0.3.1 adds two new RPCs to get all token balances of the wallet and to list all token balances associated with every address of the wallet:

---

#### counos_getwalletbalances

Returns a list of the total token balances of the whole wallet.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `includewatchonly`  | boolean | optional | include balances of watchonly addresses (default: false)                                     |

**Result:**
```js
[                           // (array of JSON objects)
  {
    "propertyid" : n,         // (number) the property identifier
    "name" : "name",            // (string) the name of the token
    "balance" : "n.nnnnnnnn",   // (string) the total available balance for the token
    "reserved" : "n.nnnnnnnn"   // (string) the total amount reserved by sell offers and accepts
    "frozen" : "n.nnnnnnnn"     // (string) the total amount frozen by the issuer
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getwalletbalances"
```

---

### counos_getwalletaddressbalances

Returns a list of all token balances for every wallet address.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `includewatchonly`  | boolean | optional | include balances of watchonly addresses (default: false)                                     |

**Result:**
```js
[                           // (array of JSON objects)
  {
    "address" : "address",      // (string) the address linked to the following balances
    "balances" :
    [
      {
        "propertyid" : n,         // (number) the property identifier
        "name" : "name",            // (string) the name of the token
        "balance" : "n.nnnnnnnn",   // (string) the available balance for the token
        "reserved" : "n.nnnnnnnn"   // (string) the amount reserved by sell offers and accepts
        "frozen" : "n.nnnnnnnn"     // (string) the amount frozen by the issuer
      },
      ...
    ]
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getwalletaddressbalances"
```

---

Information about freeze transactions added to "counos_gettransaction"
--------------------------------------------------------------

The RPC "counos_gettransaction" now has support for the new transaction types for freezing and unfreezing tokens, which were added in the last major release.

Fix behavior of "counos_listtransactions"
---------------------------------------

Previously, when trying to skip transactions with "counos_listtransactions", the list of transactions wasn't cut properly.

This behavior was fixed and proper cutting of the result is done.

Add field "issuerbonustokens" to "counos_getcrowdsale"
----------------------------------------------------

While the field "addedissuertokens" of the RPC "counos_getcrowdsale" shows the amount of issuer bonus tokens not yet emitted, the new field "issuerbonustokens" now also shows the amount of tokens already granted to the issuer as bonus of a crowdsale.

Show receiving destination, when sending to self
------------------------------------------------

When no explicit recipient is given in transactions such as Simple Sends, the transactions are considered as send-to-self, and tokens are simply sent to the original sender. This is now also reflected on the RPC layer, which previously showed a blank field as recipient.

Always show frozen balance in balance RPCs
------------------------------------------

When queriying balances, previously the field "frozen" was only shown, when there were actually frozen tokens. The field is now always returned, even if there are no frozen tokens, to make an integration easier and more foreseeable.

Add "name" field to "counos_getallbalancesforaddress"
---------------------------------------------------

This release adds the "name" field to the output of the RPC "counos_getallbalancesforaddress".

While token namens are by no way unique, or serve as identifier of a token, providing the name nevertheless can improve the user experience, because it may then no longer necessary to use a RPC like "counos_getproperty" to retrieve the name of a token.

Massive performance improvements of counoscored
---------------------------------------------

Due to optimizations of counoscored, the daemon of Counos Core, which serves as backend for exchanges and other integrators, the time to scan and parse new blocks for Counos Layer transactions was massively improved. On a regular machine, the time to process a full block could have taken up to 1.5 seconds, which was reduced in this release to about 300 ms.

Storage of state during initial scanning
----------------------------------------

Currently the state of the Counos Layer is persisted for the last 50 blocks away from the chain tip.

This is fine in most cases, but during a reparse, when the chain is fully synchronized, no state is stored until the chain tip is reached. This is an issue, if the client is shutdown during the reparse, because it must then start from the beginning.

To avoid this, the state is permanently stored every 10000 blocks. When there is also an inconsistency, in particular because one or more state files are missing, the blocks are reverted until the next previous point. In practice, this may look like this:

![image](https://user-images.githubusercontent.com/5836089/39111870-e8e0c2c6-46d6-11e8-9ef0-ca6184c41eed.png)

Additionally state is no longer persisted before the first Counos Layer transaction was mined, which speeds up the initial synchronization up to this point.

Properly restore state, when rolling back blocks
------------------------------------------------

There was an issue, which caused the client to reparse all Counos Layer transactions from the very first one, when blocks where rolled back, e.g. due to corrupted persistence files. This is a very time-consuming task and not necessary. In this release, the behavior was fixed and state is properly rolled back up to 50 blocks in the past, which significantly improves robustness of the client.

Avoid deadlock, when parsing transactions
-----------------------------------------

There was an edge case, which could have resulted in a deadlock, freezing the client, when a new block was processed and RPC queries were executed at the same time. This expressed itself as seemingly random halts of the program, especially in an environment with many frequent RPC queries.

This, and the former improvements, make synchronizing a new Counos Core node, as well as maintaining one, much more frictionless.

Update checkpoints up to block 520000
-------------------------------------

Two new consensus state checkpoints were added to this release, to ensure the user has no faulty database.

Internal preparations for native Segregated Witness support
-----------------------------------------------------------

Counos Core and the Counos Layer support Segregated Witness scripts wrapped as script hash since the beginning, which can provide a significant cost saving.

However, there is no support for native Segregated Witness scripts, which are idenifiable by their bech32 encoding, yet.

This release includes internal preparations for native Segregated Witness scripts to pave the way for full support.

Improved internal database structure
------------------------------------

A huge improvement of the internal database file structure was done in this release.

In particular database, rpc and wallet files are grouped by a common filename-prefix. This provides a better handling of the project and shows which files are related and which are not.

Change log
==========

The following list includes relevant pull requests merged into this release:
```
- #520 Move, group and rename functions and files for better architecture
- #521 Update version to 0.3.99 to indicate development
- #567 Fix JSON input conversions, remove checks, when creating raw payloads
- #568 Add freezing transaction data to counos_gettransaction
- #576 Add screenshot and wiki to README
- #581 No longer print Exodus balance after startup
- #592 Fix cutting of counos_listtransactions
- #594 Add support for native SW to safe solver
- #596 Update checkpoints up until block 510000
- #593 Make parsing more robust by persisting state every 10000 blocks
- #630 Update checkpoint for block 520000
- #634 Fix fail safe iteration when forming ECDSA point
- #649 Fix .gitignore
- #650 Add field "issuerbonustokens" to "counos_getcrowdsale"
- #651 Update default value for end block of counos_listtransactions
- #658 Add API to create funded raw transactions
- #690 Fix erasing from persistence set
- #693 Make boost::multi_index comparators const
- #695 Fix documentation for "counos_sendissuancefixed" RPC
- #697 Move setting properties of send-to-selfs into parsing
- #711 Avoid deadlock, when parsing transcations
- #713 Add and update fields for RPC help
- #715 Skip wallet balance caching, when not in UI mode
- #722 Always show frozen balance in balance RPCs
- #724 Add RPC documentation for creating funded transactions
- #723 Add name field to counos_getallbalancesforaddress
- #716 Add two new RPCs to retrieve wallet balance information
- #725 Add table of contents to RPC documentation
- #727 Fix skipping balances, when using counos_getallbalancesforaddress
- #728 Sign and broadcast funded transactions in one go
- #741 Remove old reference to renamed file
- #744 Clarify that counoshs are meant in funded RPCs
- #631 Bump version to Counos Core 0.3.1
- #632 Add release notes for Counos Core 0.3.1
- #746 Fix typo in release notes
```

Credits
=======

Thanks to everyone who contributed to this release, and especially @lxjxiaojun, @fengprofile and @vagabondanthe for their valuable contributions!
