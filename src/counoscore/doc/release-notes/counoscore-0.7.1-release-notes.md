Counos Core v0.7.1
================

Counos Core 0.7.1 paves the way for trading any asset on the Counos Layer for CounosH in a decentralized fashion. It adds new commands to accept and execute orders on the distributed exchange.

Please note, if you have not yet upgrade from 0.6 or earlier versions: Counos Core 0.7 changed the code base of Counos Core from CounosH Core 0.13.2 to CounosH Core 0.18.1. Once consensus affecting features are enabled, this version is no longer compatible with previous versions and an upgrade is required. Due to the upgrade from CounosH Core 0.13.2 to 0.18.1, this version incooperates many changes, so please take your time to read through all previous release notes carefully. The first time you upgrade from Counos Core 0.6 or older versions, the database is reconstructed, which can easily consume several hours. An upgrade from 0.7 to 0.7.1 is seamless.

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues


Table of contents
=================

- [Counos Core v0.7.1](#counos-core-v071)
- [Upgrading and downgrading](#upgrading-and-downgrading)
  - [How to upgrade](#how-to-upgrade)
  - [Downgrading](#downgrading)
  - [Compatibility with CounosH Core](#compatibility-with-counosh-core)
- [Imported changes and notes](#imported-changes-and-notes)
  - [Allow any token to be traded for CounosH](#allow-any-token-to-be-traded-for-counosh)
  - [Updates to counos_senddexaccept to accept orders](#updates-to-counos_senddexaccept-to-accept-orders)
  - [New command counos_senddexpay to execute orders](#new-command-counos_senddexpay-to-execute-orders)
  - [rpcallowip option changes](#rpcallowip-option-changes)
  - [Updates to the Counos Core logo](#updates-to-the-counos-core-logo)
  - [Several stability and performance improvements](#several-stability-and-performance-improvements)
- [Change log](#change-log)
- [Credits](#credits)


Upgrading and downgrading
=========================

How to upgrade
--------------

If you are running CounosH Core or an older version of Counos Core, shut it down. Wait until it has completely shut down, then copy the new version of `counoscored`, `counoscore-cli` and `counoscore-qt`. On Microsoft Windows the setup routine can be used to automate these steps.

When upgrading from an older version, the database is reconstructed, which can easily consume several hours.

During the first startup historical Counos transactions are reprocessed and Counos Core will not be usable for approximately 15 minutes up to two hours. The progress of the initial scan is reported on the console, the GUI and written to the `debug.log`. The scan may be interrupted, but can not be resumed, and then needs to start from the beginning.

Downgrading
-----------

Downgrading to an Counos Core version prior to 0.7.0 is not supported.

Compatibility with CounosH Core
-------------------------------

Counos Core is based on CounosH Core 0.18.1 and can be used as replacement for CounosH Core. Switching between Counos Core and CounosH Core may be supported.

However, it is not advised to upgrade or downgrade to versions other than CounosH Core 0.18. When switching to Counos Core, it may be necessary to reprocess Counos Layer transactions.


Imported changes and notes
==========================

Allow any token to be traded for CounosH
----------------------------------------

Right now the native distributed exchange of the Counos Layer protocol supports trading Counos and Test Counos for CounosH.

With this version, any token can be traded and there are no longer any trading restrictions. Please see the documentation for the related RPCs:

- [counos_senddexsell](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/counoscore/doc/rpc-api.md#counos_senddexsell)
- [counos_senddexaccept](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/counoscore/doc/rpc-api.md#counos_senddexaccept)
- [counos_senddexpay](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/counoscore/doc/rpc-api.md#counos_senddexpay)
- [counos_getactivedexsells](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/counoscore/doc/rpc-api.md#counos_getactivedexsells)

As well as the specification of the Counos Layer protocol:

- [Counos Layer protocol: 8.2. Distributed Exchange](https://github.com/CounosLayer/spec/#82-distributed-exchange)

Please note: this consensus change is not yet activated, but included in this release. An announcement will be made, when this feature is activated.


Updates to `counos_senddexaccept` to accept orders
------------------------------------------------

The RPC `counos_senddexaccept` was updated to properly pay transaction fees, when accepting an offer on the distributed exchange.

### counos_senddexaccept

Create and broadcast an accept offer for the specified token and amount.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the seller                                                                    |
| `propertyid`        | number  | required | the identifier of the token to purchase                                                      |
| `amount`            | string  | required | the amount to accept                                                                         |
| `override`          | boolean | required | override minimum accept fee and payment window checks (use with caution!)                    |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_senddexaccept" \
    "35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "15.0"
```

---


New command `counos_senddexpay` to execute orders
-----------------------------------------------

To pay for an offer after it was successfully accepted, a new command `counos_senddexpay` was added.

### counos_senddexpay

Create and broadcast payment for an accept offer.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the seller                                                                    |
| `propertyid`        | number  | required | the identifier of the token to purchase                                                      |
| `amount`            | string  | required | the CounosH amount to send                                                                   |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_senddexpay" \
    "35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "15.0"
```

---


`rpcallowip` option changes
------------------------

The `rpcallowip` option can no longer be used to automatically listen on all network interfaces.  Instead, the `rpcbind` parameter must be used to specify the IP addresses to listen on. Listening for RPC commands over a public network connection is insecure and should be disabled, so a warning is now printed if a user selects such a configuration.  If you need to expose RPC in order to use a tool like  Docker, ensure you only bind RPC to your localhost, e.g. `docker run [...] -p 127.0.0.1:8332:8332` (this is an extra `:8332` over the normal Docker port specification).


Updates to the Counos Core logo
-----------------------------

To show the synergy between the Counos Layer protocol and CounosH, the Counos Core logos were slightly changed to also include the CounosH logo:

- [Counos Core mainnet icon](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/qt/res/icons/counosh.png)
- [Counos Core testnet icon](https://github.com/CounosLayer/counoscore/blob/v0.7.1/src/qt/res/icons/counosh_testnet.png)


Several stability and performance improvements
----------------------------------------------

In some rare cases locking issues may have caused the client to halt. This version comes with several stability and performance improvements to resolve these issues.


Change log
==========

The following list includes relevant pull requests merged into this release:

```
- #1048 counos_senddexaccept pass min fee to CreateTransaction
- #1052 Add RPC DEx call to pay for accepted offer
- #1054 Debug and concurrency updates
- #1060 Only use wtxNew if fSuccess true
- #1064 New icons and default splash
- #1066 Prepare release of Counos Core v0.7.1
- #1068 Update release notes for v0.7.1
```


Credits
=======

Thanks to everyone who contributed to this release, especially to Peter Bushnell and all CounosH Core developers.
