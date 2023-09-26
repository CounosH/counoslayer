Counos Core v0.2.0
===================

v0.2.0 is a release with minor changes and improvements based on CounosH Core 0.13.2.

This version is built on top of v0.0.12, which is a major release and consensus critical in terms of the Counos Layer protocol rules. If you are using an older version of Counos Core than v0.0.12, an upgrade is mandatory, and highly recommended. Prior releases will not be compatible with new behavior in this release.

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues

Table of contents
=================

- [Counos Core v0.2.0](#counos-core-v020)
- [Upgrading and downgrading](#upgrading-and-downgrading)
  - [How to upgrade](#how-to-upgrade)
  - [Downgrading](#downgrading)
  - [Compatibility with CounosH Core](#compatibility-with-counosh-core)
- [Imported changes and notes](#imported-changes-and-notes)
  - [Upgrade to CounosH Core 0.13.2](#upgrade-to-counosh-core-0132)
  - [Important transaction fee behavior changes](#important-transaction-fee-behavior-changes)
  - [API changes](#api-changes)
  - [New project versioning scheme](#new-project-versioning-scheme)
  - [New project branch structure](#new-project-branch-structure)
- [Consensus affecting changes](#consensus-affecting-changes)
    - [Fee distribution system on the Distributed Exchange](#fee-distribution-system-on-the-distributed-exchange)
    - [Send to Owners cross property support](#send-to-owners-cross-property-support)
- [Notable changes](#notable-changes)
  - [Avoid selection of uneconomic UTXO during transaction creation](#avoid-selection-of-uneconomic-utxo-during-transaction-creation)
  - [Performance improvements during initial parsing](#performance-improvements-during-initial-parsing)
  - [New checkpoints and seed blocks up to block 460,000](#new-checkpoints-and-seed-blocks-up-to-block-460000)
  - [Easy access to specific consensus hashes when parsing](#easy-access-to-specific-consensus-hashes-when-parsing)
  - [Various bug fixes and improvements](#various-bug-fixes-and-improvements)
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

Downgrading to an Counos Core version prior 0.0.12 is generally not supported as older versions will not provide accurate information due to the changes in consensus rules. Downgrading to Counos Core 0.0.12 can require a reindex of the blockchain, and is not recommended.

Compatibility with CounosH Core
-------------------------------

Counos Core is based on CounosH Core 0.13.2 and can be used as replacement for CounosH Core. Switching between Counos Core and CounosH Core is fully supported at any time.

Downgrading to a CounosH Core version prior 0.12 may not be supported due to the obfuscation of the blockchain database.

Downgrading to a CounosH Core version prior 0.10 is not supported due to the new headers-first synchronization.

Imported changes and notes
==========================

Upgrade to CounosH Core 0.13.2
------------------------------

The underlying base of Counos Core was upgraded from CounosH Core 0.10.4 to CounosH Core 0.13.2.

Please see the following release notes for further details:

- [Release notes for CounosH Core 0.11.0](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.11.0.md)
- [Release notes for CounosH Core 0.11.1](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.11.1.md)
- [Release notes for CounosH Core 0.11.2](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.11.2.md)
- [Release notes for CounosH Core 0.12.0](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.12.0.md)
- [Release notes for CounosH Core 0.12.1](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.12.1.md)
- [Release notes for CounosH Core 0.13.0](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.13.0.md)
- [Release notes for CounosH Core 0.13.1](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.13.1.md)
- [Release notes for CounosH Core 0.13.2](https://github.com/counosh/counosh/blob/master/doc/release-notes/release-notes-0.13.2.md)

Important transaction fee behavior changes
------------------------------------------

Earlier versions of Counos Core (prior to 0.2.0) used CounosH Core 0.10.x as a base. Counos Core 0.2.0 however is based on a much newer version of CounosH Core (0.13.2) and thus inherits various changes and improvements to the handling of fees that have been added to CounosH Core over time.

It is highly recommended that users of Counos Core consider these fee changes and their chosen fee settings when upgrading to Counos Core 0.2.0, and test thoroughly to ensure that fee behavior is desirable and as expected.

Consideration of the modified behavior for the `-paytxfee` setting is especially important. Earlier versions of CounosH Core (and thus earlier versions of Counos Core prior to 0.2.0) would round the size of the transaction upwards to the nearest kilobyte when calculating the fee (for example a 250 byte transaction would be rounded up to 1 kB). This issue has been resolved in newer versions of CounosH Core, and so Counos Core 0.2.0 will no longer perform this rounding when calculating the fee. A comparison of the behaviors can be shown in the following, where an example `paytxfee` value of 0.001 CCH/kB has been set:

- Counos Core prior to 0.2.0: A transaction with a size of 250 bytes will be rounded up to 1 kB, and so a fee of 0.001 CCH will be used
- Counos Core 0.2.0: A transaction with a size of 250 bytes will not be rounded, and so a fee of 0.00025 CCH will be used

It is also worth noting that the fee estimation algorithms were updated, and thus the fees chosen when using `-txconfirmtarget` (along with the output of the `estimatefee` RPC) will likely be different in Counos Core 0.2.0 when compared to prior versions.

API changes
-----------

The behavior of the RPC [counos_gettradehistoryforaddress](https://github.com/CounosLayer/counoscore/blob/master/src/counoscore/doc/rpc-api.md#counos_gettradehistoryforaddress) was amended to return newest transactions first instead of oldest.

New project versioning scheme
-----------------------------

Starting with this version of Counos Core, the versioning scheme becomes more intuitive, as it uses MAJOR.MINOR.PATCH from now on, whereby MAJOR indicates consensus affecting changes or other non-backwards-compatible changes, MINOR indicates new functionality in a backwards-compatible manner, and PATCH is used to indicate backwards-compatible bug fixes.

New project branch structure
----------------------------

The latest stable version of Counos Core can be found in the [master](https://github.com/CounosLayer/counoscore/tree/master) branch on GitHub, while the version under development can be found in the [develop](https://github.com/CounosLayer/counoscore/tree/develop) branch. This provides a cleaner separation of source code.


Consensus affecting changes
===========================

All changes of the consensus rules are enabled by activation transactions.

Please note, while Counos Core 0.2.0 contains support for several new rules and features they are not enabled immediately and will be activated via the feature activation mechanism described above.

It follows an overview and a description of the consensus rule changes:

Fee distribution system on the Distributed Exchange
---------------------------------------------------

Counos Core 0.2.0 contains a fee caching and distribution system. This system collects small amounts of tokens in a cache until a distribution threshold is reached.  Once this distribution threshold (trigger) is reached for a property, the fees in the cache will be distributed proportionally to holders of the Counos (#1) and Test-Counos (#2) tokens based on the percentage of the total Counos tokens owned.

Once activated fees will be collected from trading of non-Counos pairs on the Distributed Exchange (there is no fee for trading Counos pairs).  The party removing liquidity from the market will incur a 0.05% fee which will be transferred to the fee cache, and subsequently distributed to holders of the Counos token.

- Placing a trade where one side of the pair is Counos (#1) or Test-Counos (#2) incurs no fee
- Placing a trade where liquidity is added to the market (i.e. the trade does not immediately execute an existing trade) incurs no fee
- Placing a trade where liquidity is removed from the market (i.e. the trade immediately executes an existing trade) the liquidity taker incurs a 0.05% fee

See also [fee system JSON-RPC API documentation](https://github.com/CounosLayer/counoscore/blob/master/src/counoscore/doc/rpc-api.md#fee-system).

This change is identified by `"featureid": 9` and labeled by the GUI as `"Fee system (inc 0.05% fee from trades of non-Counos pairs)"`.

Send To Owners cross property support
-------------------------------------

Once activated distributing tokens via the Send To Owners transaction will be permitted to cross properties if using version 1 of the transaction.

Tokens of property X then may be distributed to holders of property Y.

There is a significantly increased fee (0.00001000 per recipient) for using version 1 of the STO transaction.  The fee remains the same (0.00000001) per recipient for using version 0 of the STO transaction.

Sending an STO transaction via Counos Core that distributes tokens to holders of the same property will automatically be sent as version 0, and sending a cross-property STO will automatically be sent as version 1.

The transaction format of new Send To Owners version is as follows:

| **Field**                      | **Type**        | **Example** |
| ------------------------------ | --------------- | ----------- |
| Transaction version            | 16-bit unsigned | 65535       |
| Transaction type               | 16-bit unsigned | 65534       |
| Tokens to transfer             | 32-bit unsigned | 6           |
| Amount to transfer             | 64-bit signed   | 700009      |
| Token holders to distribute to | 32-bit unsigned | 23          |

This change is identified by `"featureid": 10` and labeled by the GUI as `"Cross-property Send To Owners"`.

Notable changes
===============

Avoid selection of uneconomic UTXO during transaction creation
--------------------------------------------------------------

In earlier version of Counos Core (prior to 0.2.0), when creating transactions with the Qt UI or the JSON-RPC API (for example with `counos_send`), then the coin selection algorithm may have selected unspent outputs, which are not economic to spend. This may have caused the creation of larger and more expensive transactions than necessary.

In Counos Core 0.2.0 this is addressed by excluding inputs during the transaction creation, which are more expensive to spend than they are worth. Please note the exclusion is directly related to the fee related configuration options of Counos Core, such as `-paytxfee` or `-txconfirmtarget`.

Performance improvements during initial parsing
-----------------------------------------------

Due to various improvements and optimizations, the initial parsing process, when running Counos Core the first time, or when starting Counos Core with `-startclean` flag, is faster by a factor of up to 10x. The improvements also have a positive impact on the time, when processing a new block.

New checkpoints and seed blocks up to block 460,000
---------------------------------------------------

To further speed up the initial parsing process, blocks without Counos transactions are skipped up until block 460,000. To avoid relying on a hardcoded list of seed blocks, Counos Core can be started with `-counosseedblockfilter=0`.

Easy access to specific consensus hashes when parsing
-----------------------------------------------------

Previously to confirm a consensus hash for a particular block it was required to enable `-counosdebug=consensus_hash_every_block` during parsing to log the hash for every block which caused a significant slow down due to the extra work involved.

This leads to circumstances where to validate a single consensus hash it is necessary to perform vastly more work than necessary.

This version adds a `-counosshowblockconsensushash` startup option which can be used to generate consensus hashes for specific blocks.

For example, to validate the checkpoint for block 450,000 without using seed block filtering we can use:

```
./counoscored --startclean --counosseedblockfilter=false --counosshowblockconsensushash=450000
```

Which will then cause a consensus hash to be generated for the corresponding block and written to the log. Multiple instances of the parameter can be used to specify multiple blocks to generate consensus hashes for.

Various bug fixes and improvements
----------------------------------

Various smaller improvements were added Counos Core 0.2.0, such as:

- Fix incorrect value from getTotalTokens when fees are cached
- Remove forwarding of setgenerate to generate
- Reduce test time to avoid hitting Travis CI time limit
- Sanitize RPC responses and replace non-UTF-8 compliant characters
- Set minimum fee distribution threshold and protect against empty distributions
- Check for fee distribution when total number of tokens is changed
- Fix missing include of test utils header
- Fix two Counos Core related build warnings
- Automatically remove stale pending transactions
- Relax data type checks of counos_createrawtx_change
- Fix possible lock contention in counos_getactivedexsells
- Remove managed property check in Change Issuer RPC
- Hardcode activations up to block 438500
- Fix a number of bugs in the QT UI
- Lock fetching and processing inputs while parsing
- Run RPC tests with explicitly defined datadir and minimum logging

Change log
==========

The following list includes relevant pull requests merged into this release:
```
- #436 Improve parsing performance
- #439 Fix incorrect value from getTotalTokens when fees are cached
- #440 Remove forwarding of setgenerate to generate
- #441 Reduce test time to avoid hitting Travis CI time limit
- #443 Sanitize RPC responses and replace non-UTF-8 compliant characters
- #447 Set minimum fee distribution threshold and protect against empty distributions
- #448 Check for fee distribution when total number of tokens is changed
- #451 Fix missing include of test utils header
- #450 Port code base to CounosH Core 0.13.2
- #453 Update splash screen to be similar to 0.0.11
- #454 Fix two Counos Core related build warnings
- #458 Add checkpoint for block 450,000
- #457 Add seed blocks for 440,000 to 450,000
- #456 Provide easy access to specific consensus hashes when parsing
- #460 Show newest transactions for counos_gettradehistoryforaddress
- #463 Automatically remove stale pending transactions
- #464 Relax data type checks of counos_createrawtx_change
- #465 Fix possible lock contention in counos_getactivedexsells
- #466 Add consensus hash for block 460,000
- #467 Add seed blocks for 450,000 to 460,000
- #468 Remove managed property check in Change Issuer RPC
- #470 Hardcode activations up to block 438500
- #471 Fix a number of bugs in the QT UI
- #472 Lock fetching and processing inputs while parsing
- #473 Avoid selecting uneconomic UTXO during transaction creation
- #474 Run RPC tests with explicitly defined datadir and minimum logging
- #460 Bump version to Counos Core 0.2.0 and add release notes
```

Credits
=======

Thanks to everyone who contributed to this release, and especially the CounosH Core developers for providing the foundation for Counos Core!
