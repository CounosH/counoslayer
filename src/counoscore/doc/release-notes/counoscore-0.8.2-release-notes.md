Counos Core v0.8.2
================

v0.8.2 is a minor release and adds new RPCs to interact with the distributed exchange, which can be used to trade any tokens for counoshs. It also incorporates significant performance improvements for the initial synchronization and upgrading from older versions of Counos Core.

Upgrading from 0.8.1 does not require a rescan and can be done very fast without interruption.

Please report bugs using the issue tracker on GitHub:

  https://github.com/CounosLayer/counoscore/issues


Table of contents
=================

- [Counos Core v0.8.2](#counos-core-v082)
- [Upgrading and downgrading](#upgrading-and-downgrading)
  - [How to upgrade](#how-to-upgrade)
  - [Downgrading](#downgrading)
  - [Compatibility with CounosH Core](#compatibility-with-counosh-core)
- [Improvements](#improvements)
  - [New RPCs for the distributed exchange](#new-rpcs-for-the-distributed-exchange)
  - [Other RPC improvements](#other-rpc-improvements)
  - [Reporting vulnerabilities](#reporting-vulnerabilities)
- [Change log](#change-log)
- [Credits](#credits)


Upgrading and downgrading
=========================

How to upgrade
--------------

If you are running CounosH Core or an older version of Counos Core, shut it down. Wait until it has completely shut down, then copy the new version of `counoscored`, `counoscore-cli` and `counoscore-qt`. On Microsoft Windows the setup routine can be used to automate these steps.

When upgrading from an older version than 0.8.0, the database of Counos Core is reconstructed, which can easily consume several hours. During the first startup historical Counos Layer transactions are reprocessed and Counos Core will not be usable for several hours up to more than a day. The progress of the initial scan is reported on the console, the GUI and written to the `debug.log`. The scan may be interrupted and can be resumed.

Upgrading from 0.8.1 does not require a rescan and can be done very fast without interruption.

Downgrading
-----------

Downgrading to an Counos Core version prior to 0.8.0 is not supported.

Compatibility with CounosH Core
-------------------------------

Counos Core is based on CounosH Core 0.18.1 and can be used as replacement for CounosH Core. Switching between Counos Core and CounosH Core may be supported.

However, it is not advised to upgrade or downgrade to versions other than CounosH Core 0.18. When switching to Counos Core, it may be necessary to reprocess Counos Layer transactions.


Improvements
============

New RPCs for the distributed exchange
-------------------------------------

Three new RPCs were added to create, update and cancel orders on the distributed exchange:

### counos_sendnewdexorder

Creates a new sell offer on the distributed token/CCH exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `amountdesired`     | string  | required | the amount of counoshs desired                                                               |
| `paymentwindow`     | number  | required | a time limit in blocks a buyer has to pay following a successful accepting order             |
| `minacceptfee`      | string  | required | a minimum mining fee a buyer has to pay to accept the offer                                  |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendnewdexorder" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "1.5" "0.75" 50 "0.0001"
```

---

### counos_sendupdatedexorder

Updates an existing sell offer on the distributed token/CCH exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `propertyidforsale` | number  | required | the identifier of the tokens to update                                                       |
| `amountforsale`     | string  | required | the new amount of tokens to list for sale                                                    |
| `amountdesired`     | string  | required | the new amount of counoshs desired                                                           |
| `paymentwindow`     | number  | required | a new time limit in blocks a buyer has to pay following a successful accepting order         |
| `minacceptfee`      | string  | required | a new minimum mining fee a buyer has to pay to accept the offer                              |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendupdatedexorder" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "1.0" "1.75" 50 "0.0001"
```

---

### counos_sendcanceldexorder

Cancels existing sell offer on the distributed token/CCH exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `propertyidforsale` | number  | required | the identifier of the tokens to cancel                                                       |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendcanceldexorder" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1
```

---

### counos_senddexpay

Create and broadcast payment for an accept offer.

Please note:
Partial purchases are not possible and the whole accepted amount must be paid.

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


Other RPC improvements
----------------------

- Active orders on the distributed exchange are now properly listed with the `counos_getactivedexsells` RPC.
- Parsing of the `ecosystem` parameter of the `counos_sendissuancecrowdsale`, `counos_sendissuancefixed`, `counos_sendissuancemanaged`, `counos_createpayload_issuancefixed`, `counos_createpayload_issuancecrowdsale`, `counos_createpayload_issuancemanaged` RPCs was fixed.
- Parsing of the `redeemkey` parameter of the `counos_createrawtx_multisig` RPC was fixed.
- When the node is not synchronized, `counos_decodetransaction` needs to be provided with all inputs. If they are missing, a proper error message is now shown.
- The fields `issuer`, `creationtxid`, `fixedissuance` and `managedissuance` are now included, when using `counos_listproperties`.
- Spelling and language improvements.


Reporting vulnerabilities
-------------------------

A security policy was created. To see instructions on how to report a vulnerability for Counos Core the Counos Layer protocol, please see [SECURITY.md](https://github.com/CounosLayer/counoscore/blob/master/SECURITY.MD).


Change log
==========

The following list includes relevant pull requests merged into this release:

```
- #1100 doc: set ecosystem as numeric
- #1102 Fix parsing of redeemkey of counos_createrawtx_multisig
- #1103 Fix typo: depreciated -> deprecated
- #1114 Split string to maintain property ID
- #1115 Add seperate RPCs for each DEx 1 action
- #1116 Bump version to Counos Core 0.8.2
- #1118 Add error text for counos_decodetransaction, when inputs are missing
- #1119 Add more information when listing tokens
- #1120 Add security policy
- #1121 Corrections to display of DEx info
- #1125 Pay exact amount for indivisible tokens
- #1117 Update release notes for Counos Core 0.8.2
```


Credits
=======

Thanks to everyone who contributed to this release, especially to Peter Bushnell.
