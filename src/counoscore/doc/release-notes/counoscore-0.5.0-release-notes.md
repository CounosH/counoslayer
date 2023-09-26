Counos Core v0.5.0
================

v0.5.0 is a major release and consensus critical in terms of the Counos Layer protocol rules. An upgrade is mandatory, and highly recommended. Prior releases may not be compatible with new behaviour in this release.

**Note: the first time you run this version, all Counos Layer transactions are reprocessed due to an database update, which may take 30 minutes up to several hours.**

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues


Table of contents
=================

- [Counos Core v0.5.0](#counos-core-v050)
- [Upgrading and downgrading](#upgrading-and-downgrading)
  - [How to upgrade](#how-to-upgrade)
  - [Downgrading](#downgrading)
  - [Compatibility with CounosH Core](#compatibility-with-counosh-core)
- [Notable changes](#notable-changes)
  - [Fix startup issue of Counos Core](#fix-startup-issue-of-counos-core)
  - [Speed up RPC counos_listpendingtransactions](#speed-up-rpc-counos_listpendingtransactions)
  - [Rename COUNOS and TCOUNOS to COUN and TCOUN](#rename-counos-and-tcounos-to-omn-and-tomn)
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

Downgrading to an Counos Core version prior to 0.4.0 is generally not advised as older versions may not provide accurate information due to the changes in consensus rules.

Compatibility with CounosH Core
-------------------------------

Counos Core is based on CounosH Core 0.13.2 and can be used as replacement for CounosH Core. Switching between Counos Core and CounosH Core may be supported.

Upgrading to a higher CounosH Core version is generally supported, but when downgrading from CounosH Core 0.15, Counos Core needs to be started with `-reindex-chainstate` flag, to rebuild the chainstate data structures in a compatible format.

Downgrading to a CounosH Core version prior to 0.12 may not be supported due to the obfuscation of the blockchain database. In this case the database also needs to be rebuilt by starting Counos Core with `-reindex-chainstate` flag.

Downgrading to a CounosH Core version prior to 0.10 is not supported due to the new headers-first synchronization.


Notable changes
===============

Fix startup issue of Counos Core
------------------------------

During startup, when reloading the effect of freeze transactions, it is checked, whether the sender of a freeze transaction is the issuer of that token and thus allowed to freeze tokens. However, if the issuer of the token has been changed in the meantime, that check fails. Such a fail is interpreted as state inconsistency, which is critical and causes a shutdown of the client.

With this change, historical issuers are persisted and can be accessed for any given block. When there is an issuer check, it now checks against the issuer at that point, resolving the startup issue.

Please note: the internal database of Counos Core is upgraded, which triggers a reparse of Counos Layer transactions the first time this version is started. This can take between 30 minutes and a few hours of processing time, during which Counos Core is unusable!

Speed up RPC "counos_listpendingtransactions"
-------------------------------------------

When adding a transaction to the mempool, a quick and unsafe check for any Counos Layer markers is done without checking transaction validity or whether it's malformed, to identify potential Counos Layer transactions.

If the transaction has a potential marker, then it's added to a new cache. If the transaction is removed from the mempool, it's also removed from the cache.

This speeds up the RPC "counos_listpendingtransactions" significantly, which can be used to list pending Counos Layer transactions.

Rename COUNOS and TCOUNOS to COUN and TCOUN
-------------------------------------

To be more aligned with other symbols and tickers the following changes in wording are made:

- "Counos", referring to the native tokens of the Counos Layer protocol, becomes "Counos tokens"
- "Test Counos", referring to the native test tokens of the Counos Layer protocol, becomes "Test Counos tokens"
- "COUNOS", referring to the symbol of Counos tokens, becomes "COUN"
- "TCOUNOS", referring to the symbol of Test Counos tokens, becomes "TCOUN"

While this is change is mostly cosmetic - in particular it changes the code documentation, RPC help messages and RPC documentation - it also has an effect of the RPCS "counos_getproperty 1" and "counos_getproperty 2", which now return a text with the updated token and symbol names.


Change log
==========

The following list includes relevant pull requests merged into this release:

```
- #907 Update version to 0.4.0.99 to indicate development
- #910 Add marker cache to speed up counos_listpendingtransactions
- #925 Store historical issuers and use that data
- #908 Rename COUNOS and TCOUNOS to COUN and TCOUN
- #931 Bump version to Counos Core 0.5.0
- #932 Add release notes for Counos Core 0.5.0
```


Credits
=======

Thanks to everyone who contributed to this release!
