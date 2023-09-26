JSON-RPC API
============

Counos Core is a fork of CounosH Core, with Counos Protocol feature support added as a new layer of functionality on top. As such interacting with the API is done in the same manner (JSON-RPC) as CounosH Core, simply with additional RPCs available for utilizing Counos Protocol features.

As all existing CounosH Core functionality is inherent to Counos Core, the RPC port by default remains as `8332` as per CounosH Core.  If you wish to run Counos Core in tandem with CounosH Core (eg. via a separate datadir) you may utilize the `-rpcport<port>` option to nominate an alternative port number.

All available commands can be listed with `"help"`, and information about a specific command can be retrieved with `"help <command>"`.

*Please note: this document may not always be up-to-date. There may be errors, omissions or inaccuracies present.*


## Table of contents

- [Transaction creation](#transaction-creation)
  - [counos_send](#counos_send)
  - [counos_sendnewdexorder](#counos_sendnewdexorder)
  - [counos_sendupdatedexorder](#counos_sendupdatedexorder)
  - [counos_sendcanceldexorder](#counos_sendcanceldexorder)
  - [counos_senddexaccept](#counos_senddexaccept)
  - [counos_senddexpay](#counos_senddexpay)
  - [counos_sendissuancecrowdsale](#counos_sendissuancecrowdsale)
  - [counos_sendissuancefixed](#counos_sendissuancefixed)
  - [counos_sendissuancemanaged](#counos_sendissuancemanaged)
  - [counos_sendsto](#counos_sendsto)
  - [counos_sendgrant](#counos_sendgrant)
  - [counos_sendrevoke](#counos_sendrevoke)
  - [counos_sendclosecrowdsale](#counos_sendclosecrowdsale)
  - [counos_sendtrade](#counos_sendtrade)
  - [counos_sendcanceltradesbyprice](#counos_sendcanceltradesbyprice)
  - [counos_sendcanceltradesbypair](#counos_sendcanceltradesbypair)
  - [counos_sendcancelalltrades](#counos_sendcancelalltrades)
  - [counos_sendchangeissuer](#counos_sendchangeissuer)
  - [counos_sendall](#counos_sendall)
  - [counos_sendenablefreezing](#counos_sendenablefreezing)
  - [counos_senddisablefreezing](#counos_senddisablefreezing)
  - [counos_sendfreeze](#counos_sendfreeze)
  - [counos_sendunfreeze](#counos_sendunfreeze)
  - [counos_sendadddelegate](#counos_sendadddelegate)
  - [counos_sendremovedelegate](#counos_sendremovedelegate)
  - [counos_sendanydata](#counos_sendanydata)
  - [counos_sendrawtx](#counos_sendrawtx)
  - [counos_funded_send](#counos_funded_send)
  - [counos_funded_sendall](#counos_funded_sendall)
  - [counos_sendnonfungible](#counos_sendnonfungible)
  - [counos_setnonfungibledata](#counos_setnonfungibledata)
- [Data retrieval](#data-retrieval)
  - [counos_getinfo](#counos_getinfo)
  - [counos_getbalance](#counos_getbalance)
  - [counos_getallbalancesforid](#counos_getallbalancesforid)
  - [counos_getallbalancesforaddress](#counos_getallbalancesforaddress)
  - [counos_getwalletbalances](#counos_getwalletbalances)
  - [counos_getwalletaddressbalances](#counos_getwalletaddressbalances)
  - [counos_gettransaction](#counos_gettransaction)
  - [counos_listtransactions](#counos_listtransactions)
  - [counos_listblocktransactions](#counos_listblocktransactions)
  - [counos_listblockstransactions](#counos_listblockstransactions)
  - [counos_listpendingtransactions](#counos_listpendingtransactions)
  - [counos_getactivedexsells](#counos_getactivedexsells)
  - [counos_listproperties](#counos_listproperties)
  - [counos_getproperty](#counos_getproperty)
  - [counos_getactivecrowdsales](#counos_getactivecrowdsales)
  - [counos_getcrowdsale](#counos_getcrowdsale)
  - [counos_getgrants](#counos_getgrants)
  - [counos_getsto](#counos_getsto)
  - [counos_gettrade](#counos_gettrade)
  - [counos_getorderbook](#counos_getorderbook)
  - [counos_gettradehistoryforpair](#counos_gettradehistoryforpair)
  - [counos_gettradehistoryforaddress](#counos_gettradehistoryforaddress)
  - [counos_getactivations](#counos_getactivations)
  - [counos_getpayload](#counos_getpayload)
  - [counos_getseedblocks](#counos_getseedblocks)
  - [counos_getcurrentconsensushash](#counos_getcurrentconsensushash)
  - [counos_getnonfungibletokens](#counos_getnonfungibletokens)
  - [counos_getnonfungibletokendata](#counos_getnonfungibletokendata)
  - [counos_getnonfungibletokenranges](#counos_getnonfungibletokenranges)
- [Data retrieval (address index)](#data-retrieval-address-index)
  - [getaddresstxids](#getaddresstxids)
  - [getaddressdeltas](#getaddressdeltas)
  - [getaddressbalance](#getaddressbalance)
  - [getaddressutxos](#getaddressutxos)
  - [getaddressmempool](#getaddressmempool)
  - [getblockhashes](#getblockhashes)
  - [getspentinfo](#getspentinfo)
- [Raw transactions](#raw-transactions)
  - [counos_decodetransaction](#counos_decodetransaction)
  - [counos_createrawtx_opreturn](#counos_createrawtx_opreturn)
  - [counos_createrawtx_multisig](#counos_createrawtx_multisig)
  - [counos_createrawtx_input](#counos_createrawtx_input)
  - [counos_createrawtx_reference](#counos_createrawtx_reference)
  - [counos_createrawtx_change](#counos_createrawtx_change)
  - [counos_createpayload_simplesend](#counos_createpayload_simplesend)
  - [counos_createpayload_sendall](#counos_createpayload_sendall)
  - [counos_createpayload_dexsell](#counos_createpayload_dexsell)
  - [counos_createpayload_dexaccept](#counos_createpayload_dexaccept)
  - [counos_createpayload_sto](#counos_createpayload_sto)
  - [counos_createpayload_issuancefixed](#counos_createpayload_issuancefixed)
  - [counos_createpayload_issuancecrowdsale](#counos_createpayload_issuancecrowdsale)
  - [counos_createpayload_issuancemanaged](#counos_createpayload_issuancemanaged)
  - [counos_createpayload_closecrowdsale](#counos_createpayload_closecrowdsale)
  - [counos_createpayload_grant](#counos_createpayload_grant)
  - [counos_createpayload_revoke](#counos_createpayload_revoke)
  - [counos_createpayload_changeissuer](#counos_createpayload_changeissuer)
  - [counos_createpayload_trade](#counos_createpayload_trade)
  - [counos_createpayload_canceltradesbyprice](#counos_createpayload_canceltradesbyprice)
  - [counos_createpayload_canceltradesbypair](#counos_createpayload_canceltradesbypair)
  - [counos_createpayload_cancelalltrades](#counos_createpayload_cancelalltrades)
  - [counos_createpayload_enablefreezing](#counos_createpayload_enablefreezing)
  - [counos_createpayload_disablefreezing](#counos_createpayload_disablefreezing)
  - [counos_createpayload_freeze](#counos_createpayload_freeze)
  - [counos_createpayload_unfreeze](#counos_createpayload_unfreeze)
  - [counos_createpayload_adddelegate](#counos_createpayload_adddelegate)
  - [counos_createpayload_removedelegate](#counos_createpayload_removedelegate)
  - [counos_createpayload_anydata](#counos_createpayload_anydata)
  - [counos_createpayload_sendnonfungible](#counos_createpayload_sendnonfungible)
  - [counos_createpayload_setnonfungibledata](#counos_createpayload_setnonfungibledata)
- [Fee system](#fee-system)
  - [counos_getfeecache](#counos_getfeecache)
  - [counos_getfeetrigger](#counos_getfeetrigger)
  - [counos_getfeeshare](#counos_getfeeshare)
  - [counos_getfeedistribution](#counos_getfeedistribution)
  - [counos_getfeedistributions](#counos_getfeedistributions)
- [Configuration](#configuration)
  - [counos_setautocommit](#counos_setautocommit)
- [Deprecated API calls](#deprecated-api-calls)

---

## Transaction creation

The RPCs for transaction creation can be used to create and broadcast Counos Protocol transactions.

A hash of the broadcasted transaction is returned as result.

### counos_send

Create and broadcast a simple send transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `amount`            | string  | required | the amount to send                                                                           |
| `redeemaddress`     | string  | optional | an address that can spend the transaction dust (sender by default)                           |
| `referenceamount`   | string  | optional | a counosh amount that is sent to the receiver (minimal by default)                           |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_send" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "100.0"
```

---

### counos_senddexsell

Place, update or cancel a sell offer on the distributed token/CCH exchange.

**Please note: this RPC is replaced by:**

- [counos_sendnewdexorder](#counos_sendnewdexorder)
- [counos_sendupdatedexorder](#counos_sendupdatedexorder)
- [counos_sendcanceldexorder](#counos_sendcanceldexorder)

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `amountdesired`     | string  | required | the amount of counoshs desired                                                               |
| `paymentwindow`     | number  | required | a time limit in blocks a buyer has to pay following a successful accepting order             |
| `minacceptfee`      | string  | required | a minimum mining fee a buyer has to pay to accept the offer                                  |
| `action`            | number  | required | the action to take (`1` for new offers, `2` to update, `3` to cancel)                        |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_senddexsell" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 1 "1.5" "0.75" 25 "0.0001" 1
```

---

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

---

### counos_sendissuancecrowdsale

Create new tokens as crowdsale.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (`1` for main ecosystem, `2` for test ecosystem)       |
| `type`              | number  | required | the type of the tokens to create: (`1` for indivisible tokens, `2` for divisible tokens)     |
| `previousid`        | number  | required | an identifier of a predecessor token (`0` for new crowdsales)                                |
| `category`          | string  | required | a category for the new tokens (can be `""`)                                                  |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be `""`)                                               |
| `name`              | string  | required | the name of the new tokens to create                                                         |
| `url`               | string  | required | an URL for further information about the new tokens (can be `""`)                            |
| `data`              | string  | required | a description for the new tokens (can be `""`)                                               |
| `propertyiddesired` | number  | required | the identifier of a token eligible to participate in the crowdsale                           |
| `tokensperunit`     | string  | required | the amount of tokens granted per unit invested in the crowdsale                              |
| `deadline`          | number  | required | the deadline of the crowdsale as Unix timestamp                                              |
| `earlybonus`        | number  | required | an early bird bonus for participants in percent per week                                     |
| `issuerpercentage`  | number  | required | a percentage of tokens that will be granted to the issuer                                    |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendissuancecrowdsale" \
    "3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo" 2 1 0 "Companies" "CounosH Mining" \
    "Quantum Miner" "" "" 2 "100" 1483228800 30 2
```

---

### counos_sendissuancefixed

Create new tokens with fixed supply.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (`1` for main ecosystem, `2` for test ecosystem)       |
| `type`              | number  | required | the type of the tokens to create: (`1` for indivisible tokens, `2` for divisible tokens)     |
| `previousid`        | number  | required | an identifier of a predecessor token (`0` for new tokens)                                    |
| `category`          | string  | required | a category for the new tokens (can be `""`)                                                  |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be `""`)                                               |
| `name`              | string  | required | the name of the new tokens to create                                                         |
| `url`               | string  | required | an URL for further information about the new tokens (can be `""`)                            |
| `data`              | string  | required | a description for the new tokens (can be `""`)                                               |
| `amount`            | string  | required | the number of tokens to create                                                               |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendissuancefixed" \
    "3Ck2kEGLJtZw9ENj2tameMCtS3HB7uRar3" 2 1 0 "Companies" "CounosH Mining" \
    "Quantum Miner" "" "" "1000000"
```

---

### counos_sendissuancemanaged

Create new tokens with manageable supply.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                            |
|---------------------|---------|----------|--------------------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                               |
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (`1` for main ecosystem, `2` for test ecosystem)                 |
| `type`              | number  | required | the type of the tokens to create: (`1` for indivisible tokens, `2` for divisible tokens, `5` for NFTs) |
| `previousid`        | number  | required | an identifier of a predecessor token (`0` for new tokens)                                              |
| `category`          | string  | required | a category for the new tokens (can be `""`)                                                            |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be `""`)                                                         |
| `name`              | string  | required | the name of the new tokens to create                                                                   |
| `url`               | string  | required | an URL for further information about the new tokens (can be `""`)                                      |
| `data`              | string  | required | a description for the new tokens (can be `""`)                                                         |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendissuancemanaged" \
    "3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH" 2 1 0 "Companies" "CounosH Mining" "Quantum Miner" "" ""
```

---

### counos_sendsto

Create and broadcast a send-to-owners transaction.

**Arguments:**

| Name                   | Type    | Presence | Description                                                                                  |
|------------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`          | string  | required | the address to send from                                                                     |
| `propertyid`           | number  | required | the identifier of the tokens to distribute                                                   |
| `amount`               | string  | required | the amount to distribute                                                                     |
| `redeemaddress`        | string  | optional | an address that can spend the transaction dust (sender by default)                           |
| `distributionproperty` | number  | optional | the identifier of the property holders to distribute to                                      |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendsto" \
    "32Z3tJccZuqQZ4PhJR2hxHC3tjgjA8cbqz" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 3 "5000"
```

---

### counos_sendgrant

Issue or grant new units of managed tokens.

**Arguments:**

| Name          | Type    | Presence | Description                                                                                  |
|---------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress` | string  | required | the address to send from                                                                     |
| `toaddress`   | string  | required | the receiver of the tokens (sender by default, can be `""`)                                  |
| `propertyid`  | number  | required | the identifier of the tokens to grant                                                        |
| `amount`      | string  | required | the amount of tokens to create                                                               |
| `grantdata`   | string  | optional | NFT only: data set in all NFTs created in this grant (default: empty)                                   |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendgrant" "3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH" "" 51 "7000"
```

---

### counos_sendrevoke

Revoke units of managed tokens.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `propertyid`        | number  | required | the identifier of the tokens to revoke                                                       |
| `amount`            | string  | required | the amount of tokens to revoke                                                               |
| `memo`              | string  | optional | a text note attached to this transaction (none by default)                                   |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendrevoke" "3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH" "" 51 "100"
```

---

### counos_sendclosecrowdsale

Manually close a crowdsale.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address associated with the crowdsale to close                                           |
| `propertyid`        | number  | required | the identifier of the crowdsale to close                                                     |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendclosecrowdsale" "3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo" 70
```

---

### counos_sendtrade

Place a trade offer on the distributed token exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to trade with                                                                    |
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `propertiddesired`  | number  | required | the identifier of the tokens desired in exchange                                             |
| `amountdesired`     | string  | required | the amount of tokens desired in exchange                                                     |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendtrade" "3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR" 31 "250.0" 1 "10.0"
```

---

### counos_sendcanceltradesbyprice

Cancel offers on the distributed token exchange with the specified price.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to trade with                                                                    |
| `propertyidforsale` | number  | required | the identifier of the tokens listed for sale                                                 |
| `amountforsale`     | string  | required | the amount of tokens to listed for sale                                                      |
| `propertiddesired`  | number  | required | the identifier of the tokens desired in exchange                                             |
| `amountdesired`     | string  | required | the amount of tokens desired in exchange                                                     |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendcanceltradesbyprice" "3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR" 31 "100.0" 1 "5.0"
```

---

### counos_sendcanceltradesbypair

Cancel all offers on the distributed token exchange with the given currency pair.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to trade with                                                                    |
| `propertyidforsale` | number  | required | the identifier of the tokens listed for sale                                                 |
| `propertiddesired`  | number  | required | the identifier of the tokens desired in exchange                                             |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendcanceltradesbypair" "3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR" 1 31
```

---

### counos_sendcancelalltrades

Cancel all offers on the distributed token exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to trade with                                                                    |
| `ecosystem`         | number  | required | the ecosystem of the offers to cancel (`1` for main ecosystem, `2` for test ecosystem)       |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendcancelalltrades" "3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR" 1
```

---

### counos_sendchangeissuer

Change the issuer on record of the given tokens.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address associated with the tokens                                                       |
| `toaddress  `       | string  | required | the address to transfer administrative control to                                            |
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendchangeissuer" \
    "1ARjWDkZ7kT9fwjPrjcQyvbXDkEySzKHwu" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs" 3
```

---

### counos_sendall

Transfers all available tokens in the given ecosystem to the recipient.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `toaddress  `       | string  | required | the address of the receiver                                                                  |
| `ecosystem`         | number  | required | the ecosystem of the tokens to send (`1` for main ecosystem, `2` for test ecosystem)         |
| `redeemaddress`     | string  | optional | an address that can spend the transaction dust (sender by default)                           |
| `referenceamount`   | string  | optional | a counosh amount that is sent to the receiver (minimal by default)                           |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendall" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 2
```

---

### counos_sendenablefreezing

Enables address freezing for a centrally managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from (must be issuer of a managed property)                              |
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendenablefreezing" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" 2
```

---

### counos_senddisablefreezing

Disables address freezing for a centrally managed property.

IMPORTANT NOTE:  Disabling freezing for a property will UNFREEZE all frozen addresses for that property!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from (must be issuer of a managed property)                              |
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_senddisablefreezing" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" 2
```

---

### counos_sendfreeze

Freeze an address for a centrally managed token.

Note: Only the issuer may freeze tokens, and only if the token is of the managed type with the freezing option enabled.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                      |
|---------------------|---------|----------|--------------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from (must be delegate or issuer of a managed property with freezing enabled |
| `toaddress`         | string  | required | the address to freeze                                                                            |
| `propertyid`        | number  | required | the identifier of the tokens to freeze                                                           |
| `amount`            | string  | required | the amount to freeze (note: currently unused, frozen addresses cannot transact the property)     |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendfreeze" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs" 2 1000
```

---

### counos_sendunfreeze

Unfreeze an address for a centrally managed token.

Note: Only the issuer may unfreeze tokens

**Arguments:**

| Name                | Type    | Presence | Description                                                                                      |
|---------------------|---------|----------|--------------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from (must be delegate or issuer of a managed property with freezing enabled |
| `toaddress`         | string  | required | the address to unfreeze                                                                          |
| `propertyid`        | number  | required | the identifier of the tokens to unfreeze                                                         |
| `amount`            | string  | required | the amount to unfreeze (note: currently unused                                                   |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendunfreeze" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs" 2 1000
```

---
### omni_sendadddelegate

Adds a delegate for the issuance of tokens of a managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the issuer of the tokens                                                                     |
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |
| `delegateaddress`   | string  | required | the new delegate                                                                             |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendadddelegate" "12GftZCQ3vwubWmRCmnfZAHdDWXj6ujenx" 21 "14TG9NsTxk2fvH8iGiFcVbquC5NPhHcFjh"
```

---

### counos_sendremovedelegate

Removes a delegate for the issuance of tokens of a managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the issuer or delegate of the tokens                                                         |
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |
| `delegateaddress`   | string  | required | the delegate to be removed                                                                   |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendremovedelegate" "12GftZCQ3vwubWmRCmnfZAHdDWXj6ujenx" 21 "14TG9NsTxk2fvH8iGiFcVbquC5NPhHcFjh"
```

---

### counos_sendanydata

Create and broadcast a transaction with an arbitrary payload.

When no receiver is specified, the sender is also considered as receiver.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `data`              | string  | required | the hex-encoded data                                                                         |
| `toaddress`         | string  | optional | the optional address of the receiver                                                                  |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendanydata" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "646578782032303230"
```

```bash
$ counoscore-cli "counos_sendanydata" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "646578782032303230" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs"
```

---

### counos_sendrawtx

Broadcasts a raw Counos Layer transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send from                                                                     |
| `rawtransaction`    | string  | required | the hex-encoded raw transaction                                                              |
| `referenceaddress`  | string  | optional | a reference address (none by default)                                                        |
| `redeemaddress`     | string  | optional | an address that can spend the transaction dust (sender by default)                           |
| `referenceamount`   | string  | optional | a counosh amount that is sent to the receiver (minimal by default)                           |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendrawtx" \
    "1MCHESTptvd2LnNp7wmr2sGTpRomteAkq8" "000000000000000100000000017d7840" \
    "1EqTta1Rt8ixAA32DuC29oukbsSWU62qAV"
```

---

### counos_funded_send

Creates and sends a funded simple send transaction.

All counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send the tokens from                                                          |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `amount`            | string  | required | the amount to send                                                                           |
| `feeaddress`        | string  | required | the address that is used for change and to pay for fees, if needed                           |

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

### counos_sendnonfungible

Create and broadcast a non-fungible send transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | required | the address to send from                                                                     |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `tokenstart`        | number  | required | the first token in the range to send                                                         |
| `tokenend`          | number  | required | the last token in the range to send                                                          |
| `redeemaddress`     | string  | optional | an address that can spend the transaction dust (sender by default)                           |
| `referenceamount`   | string  | optional | a bitcoin amount that is sent to the receiver (minimal by default)                           |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_sendnonfungible" "3M9qvHKtgARhqcMtM5cRT9VaiDJ5PSfQGY" "37FaKponF7zqoMLUjEiko25pDiuVH5YLEa" 70 1 1000
```

---

### counos_setnonfungibledata

Sets either the issuer or holder data field in a non-fungible token. Holder data can only be
updated by the token owner and issuer data can only be updated by address that created the tokens.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the property identifier                                                                      |
| `tokenstart`        | number  | required | the first token in the range to set data on                                                  |
| `tokenend`          | number  | required | the last token in the range to set data on                                                   |
| `issuer`            | boolean | required | if true issuer data set, otherwise holder data set                                           |
| `data`              | string  | required | data set as in either issuer or holder fields                                                |

**Result:**
```js
"hash"  // (string) the hex-encoded transaction hash
```

**Example:**

```bash
$ counoscore-cli "counos_setnonfungibledata" 70 50 60 true "string data"
```

---


### counos_funded_sendall

Creates and sends a transaction that transfers all available tokens in the given ecosystem to the recipient.

All counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `fromaddress`       | string  | required | the address to send the tokens from                                                          |
| `toaddress`         | string  | required | the address of the receiver                                                                  |
| `ecosystem`         | number  | required | the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)             |
| `feeaddress`        | string  | required | the address that is used for change and to pay for fees, if needed                           |

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


## Data retrieval

The RPCs for data retrieval can be used to get information about the state of the Counos ecosystem.

### counos_getinfo

Returns various state information of the client and protocol.

**Arguments:**

*None*

**Result:**
```js
Result:
{
  "counoscoreversion_int" : xxxxxxx,      // (number) client version as integer
  "counoscoreversion" : "x.x.x.x-xxx",    // (string) client version
  "mastercoreversion" : "x.x.x.x-xxx",  // (string) client version (DEPRECATED)
  "counoshcoreversion" : "x.x.x",       // (string) CounosH Core version
  "commitinfo" : "xxxxxxx",             // (string) build commit identifier
  "block" : nnnnnn,                     // (number) index of the last processed block
  "blocktime" : nnnnnnnnnn,             // (number) timestamp of the last processed block
  "blocktransactions" : nnnn,           // (number) Counos transactions found in the last processed block
  "totaltransactions" : nnnnnnnn,       // (number) Counos transactions processed in total
  "alerts" : [                          // (array of JSON objects) active protocol alert (if any)
    {
      "alerttype" : n                       // (number) alert type as integer
      "alerttype" : "xxx"                   // (string) alert type (can be "alertexpiringbyblock", "alertexpiringbyblocktime", "alertexpiringbyclientversion" or "error")
      "alertexpiry" : "nnnnnnnnnn"          // (string) expiration criteria (can refer to block height, timestamp or client version)
      "alertmessage" : "xxx"                // (string) information about the alert
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_getinfo"
```

---

### counos_getbalance

Returns the token balance for a given address and property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | required | the address                                                                                  |
| `propertyid`        | number  | required | the property identifier                                                                      |

**Result:**
```js
{
  "balance" : "n.nnnnnnnn",  // (string) the available balance of the address
  "reserved" : "n.nnnnnnnn", // (string) the amount reserved by sell offers and accepts
  "frozen" : "n.nnnnnnnn"    // (string) the amount frozen by the issuer (applies to managed properties only)
}
```

**Example:**

```bash
$ counoscore-cli "counos_getbalance", "cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam" 1
```

---

### counos_getallbalancesforid

Returns a list of token balances for a given currency or property identifier.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the property identifier                                                                      |

**Result:**
```js
[                          // (array of JSON objects)
  {
    "address" : "address",     // (string) the address
    "balance" : "n.nnnnnnnn",  // (string) the available balance of the address
    "reserved" : "n.nnnnnnnn", // (string) the amount reserved by sell offers and accepts
    "frozen" : "n.nnnnnnnn"    // (string) the amount frozen by the issuer (applies to managed properties only)
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getallbalancesforid" 1
```

---

### counos_getallbalancesforaddress

Returns a list of all token balances for a given address.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | required | the address                                                                                  |

**Result:**
```js
[                          // (array of JSON objects)
  {
    "propertyid" : n,          // (number) the property identifier
    "name" : "name",           // (string) the name of the property
    "balance" : "n.nnnnnnnn",  // (string) the available balance of the address
    "reserved" : "n.nnnnnnnn", // (string) the amount reserved by sell offers and accepts
    "frozen" : "n.nnnnnnnn"    // (string) the amount frozen by the issuer (applies to managed properties only)
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getallbalancesforaddress" "cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam"
```

---

### counos_getwalletbalances

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
    "frozen" : "n.nnnnnnnn"     // (string) the total amount frozen by the issuer (applies to managed properties only)
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
        "frozen" : "n.nnnnnnnn"     // (string) the amount frozen by the issuer (applies to managed properties only)
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

### counos_gettransaction

Get detailed information about an Counos transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `txid`              | string  | required | the hash of the transaction to lookup                                                        |

**Result:**
```js
{
  "txid" : "hash",                 // (string) the hex-encoded hash of the transaction
  "sendingaddress" : "address",    // (string) the CounosH address of the sender
  "referenceaddress" : "address",  // (string) a CounosH address used as reference (if any)
  "ismine" : true|false,           // (boolean) whether the transaction involes an address in the wallet
  "confirmations" : nnnnnnnnnn,    // (number) the number of transaction confirmations
  "fee" : "n.nnnnnnnn",            // (string) the transaction fee in counoshs
  "blocktime" : nnnnnnnnnn,        // (number) the timestamp of the block that contains the transaction
  "valid" : true|false,            // (boolean) whether the transaction is valid
  "positioninblock" : n,           // (number) the position (index) of the transaction within the block
  "version" : n,                   // (number) the transaction version
  "type_int" : n,                  // (number) the transaction type as number
  "type" : "type",                 // (string) the transaction type as string
  [...]                            // (mixed) other transaction type specific properties
}
```

**Example:**

```bash
$ counoscore-cli "counos_gettransaction" "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"
```

---

### counos_listtransactions

List wallet transactions, optionally filtered by an address and block boundaries.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `txid`              | string  | optional | address filter (default: `"*"`)                                                              |
| `count`             | number  | optional | show at most n transactions (default: `10`)                                                  |
| `skip`              | number  | optional | skip the first n transactions (default: `0`)                                                 |
| `startblock`        | number  | optional | first block to begin the search (default: `0`)                                               |
| `endblock`          | number  | optional | last block to include in the search (default: `999999999`)                                   |

**Result:**
```js
[                                // (array of JSON objects)
  {
    "txid" : "hash",                 // (string) the hex-encoded hash of the transaction
    "sendingaddress" : "address",    // (string) the CounosH address of the sender
    "referenceaddress" : "address",  // (string) a CounosH address used as reference (if any)
    "ismine" : true|false,           // (boolean) whether the transaction involves an address in the wallet
    "confirmations" : nnnnnnnnnn,    // (number) the number of transaction confirmations
    "fee" : "n.nnnnnnnn",            // (string) the transaction fee in counoshs
    "blocktime" : nnnnnnnnnn,        // (number) the timestamp of the block that contains the transaction
    "valid" : true|false,            // (boolean) whether the transaction is valid
    "positioninblock" : n,           // (number) the position (index) of the transaction within the block
    "version" : n,                   // (number) the transaction version
    "type_int" : n,                  // (number) the transaction type as number
    "type" : "type",                 // (string) the transaction type as string
    [...]                            // (mixed) other transaction type specific properties
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_listtransactions"
```

---

### counos_listblocktransactions

Lists all Counos transactions in a block.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `index`             | number  | required | the block height or block index                                                              |

**Result:**
```js
[      // (array of string)
  "hash",  // (string) the hash of the transaction
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_listblocktransactions" 279007
```

---

### counos_listblockstransactions

Lists all Counos transactions in a given range of blocks.

Note: the list of transactions is unordered and can contain invalid transactions!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `firstblock`        | number  | required | the index of the first block to consider                                                     |
| `lastblock`         | number  | required | the index of the last block to consider                                                      |

**Result:**
```js
[      // (array of string)
  "hash",  // (string) the hash of the transaction
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_counos_listblocktransactions" 279007 300000
```

---

### counos_listpendingtransactions

Returns a list of unconfirmed Counos transactions, pending in the memory pool.

Note: the validity of pending transactions is uncertain, and the state of the memory pool may change at any moment. It is recommended to check transactions after confirmation, and pending transactions should be considered as invalid.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | optional | filter results by address (default: `""` for no filter)                                      |

**Result:**
```js
[                                // (array of JSON objects)
  {
    "txid" : "hash",                 // (string) the hex-encoded hash of the transaction
    "sendingaddress" : "address",    // (string) the CounosH address of the sender
    "referenceaddress" : "address",  // (string) a CounosH address used as reference (if any)
    "ismine" : true|false,           // (boolean) whether the transaction involes an address in the wallet
    "fee" : "n.nnnnnnnn",            // (string) the transaction fee in counoshs
    "version" : n,                   // (number) the transaction version
    "type_int" : n,                  // (number) the transaction type as number
    "type" : "type",                 // (string) the transaction type as string
    [...]                            // (mixed) other transaction type specific properties
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_listpendingtransactions"
```

---

### counos_getactivedexsells

Returns currently active offers on the distributed exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | optional | address filter (default: include any)                                                        |

**Result:**
```js
[                                  // (array of JSON objects)
  {
    "txid" : "hash",                   // (string) the hash of the transaction of this offer
    "propertyid" : n,                  // (number) the identifier of the tokens for sale
    "seller" : "address",              // (string) the CounosH address of the seller
    "amountavailable" : "n.nnnnnnnn",  // (string) the number of tokens still listed for sale and currently available
    "counoshdesired" : "n.nnnnnnnn",   // (string) the number of counoshs desired in exchange
    "unitprice" : "n.nnnnnnnn" ,       // (string) the unit price (CCH/token)
    "timelimit" : nn,                  // (number) the time limit in blocks a buyer has to pay following a successful accept
    "minimumfee" : "n.nnnnnnnn",       // (string) the minimum mining fee a buyer has to pay to accept this offer
    "amountaccepted" : "n.nnnnnnnn",   // (string) the number of tokens currently reserved for pending "accept" orders
    "accepts": [                       // (array of JSON objects) a list of pending "accept" orders
      {
        "buyer" : "address",               // (string) the CounosH address of the buyer
        "block" : nnnnnn,                  // (number) the index of the block that contains the "accept" order
        "blocksleft" : nn,                 // (number) the number of blocks left to pay
        "amount" : "n.nnnnnnnn"            // (string) the amount of tokens accepted and reserved
        "amounttopay" : "n.nnnnnnnn"       // (string) the amount in counoshs needed finalize the trade
      },
      ...
    ]
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getactivedexsells"
```

---

### counos_listproperties

Lists all tokens or smart properties.

To get the total number of tokens, please use counos_getproperty.

**Arguments:**

*None*

**Result:**
```js
[                               // (array of JSON objects)
  {
    "propertyid" : n,               // (number) the identifier of the tokens
    "name" : "name",                // (string) the name of the tokens
    "category" : "category",        // (string) the category used for the tokens
    "subcategory" : "subcategory",  // (string) the subcategory used for the tokens
    "data" : "information",         // (string) additional information or a description
    "url" : "uri",                  // (string) an URI, for example pointing to a website
    "divisible" : true|false        // (boolean) whether the tokens are divisible
    "issuer" : "address",           // (string) the CounosH address of the issuer on record
    "creationtxid" : "hash",        // (string) the hex-encoded creation transaction hash
    "fixedissuance" : true|false,   // (boolean) whether the token supply is fixed
    "managedissuance" : true|false, // (boolean) whether the token supply is managed by the issuer
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_listproperties"
```

---

### counos_getproperty

Returns details for about the tokens or smart property to lookup.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens or property                                                     |

**Result:**
```js
{
  "propertyid" : n,               // (number) the identifier
  "name" : "name",                // (string) the name of the tokens
  "category" : "category",        // (string) the category used for the tokens
  "subcategory" : "subcategory",  // (string) the subcategory used for the tokens
  "data" : "information",         // (string) additional information or a description
  "url" : "uri",                  // (string) an URI, for example pointing to a website
  "divisible" : true|false,       // (boolean) whether the tokens are divisible
  "issuer" : "address",           // (string) the CounosH address of the issuer on record
  "delegate": "address",          // (string) the CounosH address of the issuance delegate, if there is one        
  "creationtxid" : "hash",        // (string) the hex-encoded creation transaction hash
  "fixedissuance" : true|false,   // (boolean) whether the token supply is fixed
  "managedissuance" : true|false, // (boolean) whether the token supply is managed by the issuer
  "freezingenabled" : true|false, // (boolean) whether freezing is enabled for the property (managed properties only)
  "totaltokens" : "n.nnnnnnnn"    // (string) the total number of tokens in existence
}
```

**Example:**

```bash
$ counoscore-cli "counos_getproperty" 3
```

---

### counos_getactivecrowdsales

Lists currently active crowdsales.

**Arguments:**

*None*

**Result:**
```js
[                                // (array of JSON objects)
  {
    "propertyid" : n,                // (number) the identifier of the crowdsale
    "name" : "name",                 // (string) the name of the tokens issued via the crowdsale
    "issuer" : "address",            // (string) the CounosH address of the issuer on record
    "propertyiddesired" : n,         // (number) the identifier of the tokens eligible to participate in the crowdsale
    "tokensperunit" : "n.nnnnnnnn",  // (string) the amount of tokens granted per unit invested in the crowdsale
    "earlybonus" : n,                // (number) an early bird bonus for participants in percent per week
    "percenttoissuer" : n,           // (number) a percentage of tokens that will be granted to the issuer
    "starttime" : nnnnnnnnnn,        // (number) the start time of the of the crowdsale as Unix timestamp
    "deadline" : nnnnnnnnnn          // (number) the deadline of the crowdsale as Unix timestamp
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getactivecrowdsales"
```

---

### counos_getcrowdsale

Returns information about a crowdsale.

**Arguments:**,

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the crowdsale                                                              |
| `verbose`           | boolean | optional | list crowdsale participants (default: `false`)                                               |

**Result:**
```js
{
  "propertyid" : n,                    // (number) the identifier of the crowdsale
  "name" : "name",                     // (string) the name of the tokens issued via the crowdsale
  "active" : true|false,               // (boolean) whether the crowdsale is still active
  "issuer" : "address",                // (string) the CounosH address of the issuer on record
  "propertyiddesired" : n,             // (number) the identifier of the tokens eligible to participate in the crowdsale
  "tokensperunit" : "n.nnnnnnnn",      // (string) the amount of tokens granted per unit invested in the crowdsale
  "earlybonus" : n,                    // (number) an early bird bonus for participants in percent per week
  "percenttoissuer" : n,               // (number) a percentage of tokens that will be granted to the issuer
  "starttime" : nnnnnnnnnn,            // (number) the start time of the of the crowdsale as Unix timestamp
  "deadline" : nnnnnnnnnn,             // (number) the deadline of the crowdsale as Unix timestamp
  "amountraised" : "n.nnnnnnnn",       // (string) the amount of tokens invested by participants
  "tokensissued" : "n.nnnnnnnn",       // (string) the total number of tokens issued via the crowdsale
  "issuerbonustokens" : "n.nnnnnnnn",  // (string) the amount of tokens granted to the issuer as bonus
  "addedissuertokens" : "n.nnnnnnnn",  // (string) the amount of issuer bonus tokens not yet emitted
  "closedearly" : true|false,          // (boolean) whether the crowdsale ended early (if not active)
  "maxtokens" : true|false,            // (boolean) whether the crowdsale ended early due to reaching the limit of max. issuable tokens (if not active)
  "endedtime" : nnnnnnnnnn,            // (number) the time when the crowdsale ended (if closed early)
  "closetx" : "hash",                  // (string) the hex-encoded hash of the transaction that closed the crowdsale (if closed manually)
  "participanttransactions": [         // (array of JSON objects) a list of crowdsale participations (if verbose=true)
    {
      "txid" : "hash",                     // (string) the hex-encoded hash of participation transaction
      "amountsent" : "n.nnnnnnnn",         // (string) the amount of tokens invested by the participant
      "participanttokens" : "n.nnnnnnnn",  // (string) the tokens granted to the participant
      "issuertokens" : "n.nnnnnnnn"        // (string) the tokens granted to the issuer as bonus
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_getcrowdsale" 3 true
```

---

### counos_getgrants

Returns information about granted and revoked units of managed tokens.

**Arguments:**

*None*

**Result:**
```js
{
  "propertyid" : n,              // (number) the identifier of the managed tokens
  "name" : "name",               // (string) the name of the tokens
  "issuer" : "address",          // (string) the CounosH address of the issuer on record
  "creationtxid" : "hash",       // (string) the hex-encoded creation transaction hash
  "totaltokens" : "n.nnnnnnnn",  // (string) the total number of tokens in existence
  "issuances": [                 // (array of JSON objects) a list of the granted and revoked tokens
    {
      "txid" : "hash",               // (string) the hash of the transaction that granted tokens
      "grant" : "n.nnnnnnnn"         // (string) the number of tokens granted by this transaction
    },
    {
      "txid" : "hash",               // (string) the hash of the transaction that revoked tokens
      "grant" : "n.nnnnnnnn"         // (string) the number of tokens revoked by this transaction
    },
    ...
  ]
}
```

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the managed tokens to lookup                                               |

**Example:**

```bash
$ counoscore-cli "counos_getgrants" 31
```

---

### counos_getsto

Get information and recipients of a send-to-owners transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `txid`              | string  | required | the hash of the transaction to lookup                                                        |
| `recipientfilter`   | string  | optional | a filter for recipients (wallet by default, `"*"` for all)                                   |

**Result:**
```js
{
  "txid" : "hash",               // (string) the hex-encoded hash of the transaction
  "sendingaddress" : "address",  // (string) the CounosH address of the sender
  "ismine" : true|false,         // (boolean) whether the transaction involes an address in the wallet
  "confirmations" : nnnnnnnnnn,  // (number) the number of transaction confirmations
  "fee" : "n.nnnnnnnn",          // (string) the transaction fee in counoshs
  "blocktime" : nnnnnnnnnn,      // (number) the timestamp of the block that contains the transaction
  "valid" : true|false,          // (boolean) whether the transaction is valid
  "positioninblock" : n,         // (number) the position (index) of the transaction within the block
  "version" : n,                 // (number) the transaction version
  "type_int" : n,                // (number) the transaction type as number
  "type" : "type",               // (string) the transaction type as string
  "propertyid" : n,              // (number) the identifier of sent tokens
  "divisible" : true|false,      // (boolean) whether the sent tokens are divisible
  "amount" : "n.nnnnnnnn",       // (string) the number of tokens sent to owners
  "totalstofee" : "n.nnnnnnnn",  // (string) the fee paid by the sender, nominated in COUN or TCOUN
  "recipients": [                // (array of JSON objects) a list of recipients
    {
      "address" : "address",         // (string) the CounosH address of the recipient
      "amount" : "n.nnnnnnnn"        // (string) the number of tokens sent to this recipient
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_getsto" "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d" "*"
```

---

### counos_gettrade

Get detailed information and trade matches for orders on the distributed token exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `txid`              | string  | required | the hash of the order to lookup                                                              |

**Result:**
```js
{
  "txid" : "hash",                              // (string) the hex-encoded hash of the transaction of the order
  "sendingaddress" : "address",                 // (string) the CounosH address of the trader
  "ismine" : true|false,                        // (boolean) whether the order involes an address in the wallet
  "confirmations" : nnnnnnnnnn,                 // (number) the number of transaction confirmations
  "fee" : "n.nnnnnnnn",                         // (string) the transaction fee in counoshs
  "blocktime" : nnnnnnnnnn,                     // (number) the timestamp of the block that contains the transaction
  "valid" : true|false,                         // (boolean) whether the transaction is valid
  "positioninblock" : n,                        // (number) the position (index) of the transaction within the block
  "version" : n,                                // (number) the transaction version
  "type_int" : n,                               // (number) the transaction type as number
  "type" : "type",                              // (string) the transaction type as string
  "propertyidforsale" : n,                      // (number) the identifier of the tokens put up for sale
  "propertyidforsaleisdivisible" : true|false,  // (boolean) whether the tokens for sale are divisible
  "amountforsale" : "n.nnnnnnnn",               // (string) the amount of tokens initially offered
  "propertyiddesired" : n,                      // (number) the identifier of the tokens desired in exchange
  "propertyiddesiredisdivisible" : true|false,  // (boolean) whether the desired tokens are divisible
  "amountdesired" : "n.nnnnnnnn",               // (string) the amount of tokens initially desired
  "unitprice" : "n.nnnnnnnnnnn..."              // (string) the unit price (shown in the property desired)
  "status" : "status"                           // (string) the status of the order ("open", "cancelled", "filled", ...)
  "canceltxid" : "hash",                        // (string) the hash of the transaction that cancelled the order (if cancelled)
  "matches": [                                  // (array of JSON objects) a list of matched orders and executed trades
    {
      "txid" : "hash",                              // (string) the hash of the transaction that was matched against
      "block" : nnnnnn,                             // (number) the index of the block that contains this transaction
      "address" : "address",                        // (string) the CounosH address of the other trader
      "amountsold" : "n.nnnnnnnn",                  // (string) the number of tokens sold in this trade
      "amountreceived" : "n.nnnnnnnn"               // (string) the number of tokens traded in exchange
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_gettrade" "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"
```

---

### counos_getorderbook

List active offers on the distributed token exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | filter orders by `propertyid` for sale                                                       |
| `propertyiddesired` | number  | optional | filter orders by `propertyiddesired`                                                        |

**Result:**
```js
[                                             // (array of JSON objects)
  {
    "address" : "address",                        // (string) the CounosH address of the trader
    "txid" : "hash",                              // (string) the hex-encoded hash of the transaction of the order
    "ecosystem" : "main"|"test",                  // (string) the ecosytem in which the order was made (if "cancel-ecosystem")
    "propertyidforsale" : n,                      // (number) the identifier of the tokens put up for sale
    "propertyidforsaleisdivisible" : true|false,  // (boolean) whether the tokens for sale are divisible
    "amountforsale" : "n.nnnnnnnn",               // (string) the amount of tokens initially offered
    "amountremaining" : "n.nnnnnnnn",             // (string) the amount of tokens still up for sale
    "propertyiddesired" : n,                      // (number) the identifier of the tokens desired in exchange
    "propertyiddesiredisdivisible" : true|false,  // (boolean) whether the desired tokens are divisible
    "amountdesired" : "n.nnnnnnnn",               // (string) the amount of tokens initially desired
    "amounttofill" : "n.nnnnnnnn",                // (string) the amount of tokens still needed to fill the offer completely
    "action" : n,                                 // (number) the action of the transaction: (1) "trade", (2) "cancel-price", (3) "cancel-pair", (4) "cancel-ecosystem"
    "block" : nnnnnn,                             // (number) the index of the block that contains the transaction
    "blocktime" : nnnnnnnnnn                      // (number) the timestamp of the block that contains the transaction
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getorderbook" 2
```

---

### counos_gettradehistoryforpair

Retrieves the history of trades on the distributed token exchange for the specified market.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the first side of the traded pair                                                            |
| `propertyidsecond`  | number  | required | the second side of the traded pair                                                           |
| `count`             | number  | optional | number of trades to retrieve (default: `10`)                                                 |

**Result:**
```js
[                                     // (array of JSON objects)
  {
    "block" : nnnnnn,                     // (number) the index of the block that contains the trade match
    "unitprice" : "n.nnnnnnnnnnn..." ,    // (string) the unit price used to execute this trade (received/sold)
    "inverseprice" : "n.nnnnnnnnnnn...",  // (string) the inverse unit price (sold/received)
    "sellertxid" : "hash",                // (string) the hash of the transaction of the seller
    "address" : "address",                // (string) the CounosH address of the seller
    "amountsold" : "n.nnnnnnnn",          // (string) the number of tokens sold in this trade
    "amountreceived" : "n.nnnnnnnn",      // (string) the number of tokens traded in exchange
    "matchingtxid" : "hash",              // (string) the hash of the transaction that was matched against
    "matchingaddress" : "address"         // (string) the CounosH address of the other party of this trade
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_gettradehistoryforpair" 1 12 500
```

---

### counos_gettradehistoryforaddress

Retrieves the history of orders on the distributed exchange for the supplied address.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | required | address to retrieve history for                                                              |
| `count`             | number  | optional | number of orders to retrieve (default: `10`)                                                 |
| `propertyid`        | number  | optional | filter by propertyid transacted (default: no filter)                                         |

**Result:**
```js
[                                             // (array of JSON objects)
  {
    "txid" : "hash",                              // (string) the hex-encoded hash of the transaction of the order
    "sendingaddress" : "address",                 // (string) the CounosH address of the trader
    "ismine" : true|false,                        // (boolean) whether the order involes an address in the wallet
    "confirmations" : nnnnnnnnnn,                 // (number) the number of transaction confirmations
    "fee" : "n.nnnnnnnn",                         // (string) the transaction fee in counoshs
    "blocktime" : nnnnnnnnnn,                     // (number) the timestamp of the block that contains the transaction
    "valid" : true|false,                         // (boolean) whether the transaction is valid
    "positioninblock" : n,                        // (number) the position (index) of the transaction within the block
    "version" : n,                                // (number) the transaction version
    "type_int" : n,                               // (number) the transaction type as number
    "type" : "type",                              // (string) the transaction type as string
    "propertyidforsale" : n,                      // (number) the identifier of the tokens put up for sale
    "propertyidforsaleisdivisible" : true|false,  // (boolean) whether the tokens for sale are divisible
    "amountforsale" : "n.nnnnnnnn",               // (string) the amount of tokens initially offered
    "propertyiddesired" : n,                      // (number) the identifier of the tokens desired in exchange
    "propertyiddesiredisdivisible" : true|false,  // (boolean) whether the desired tokens are divisible
    "amountdesired" : "n.nnnnnnnn",               // (string) the amount of tokens initially desired
    "unitprice" : "n.nnnnnnnnnnn..."              // (string) the unit price (shown in the property desired)
    "status" : "status"                           // (string) the status of the order ("open", "cancelled", "filled", ...)
    "canceltxid" : "hash",                        // (string) the hash of the transaction that cancelled the order (if cancelled)
    "matches": [                                  // (array of JSON objects) a list of matched orders and executed trades
      {
        "txid" : "hash",                              // (string) the hash of the transaction that was matched against
        "block" : nnnnnn,                             // (number) the index of the block that contains this transaction
        "address" : "address",                        // (string) the CounosH address of the other trader
        "amountsold" : "n.nnnnnnnn",                  // (string) the number of tokens sold in this trade
        "amountreceived" : "n.nnnnnnnn"               // (string) the number of tokens traded in exchange
      },
      ...
    ]
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_gettradehistoryforaddress" "1MCHESTptvd2LnNp7wmr2sGTpRomteAkq8"
```

---

### counos_getactivations

Returns pending and completed feature activations.

**Result:**
```js
{
  "pendingactivations": [      // (array of JSON objects) a list of pending feature activations
    {
      "featureid" : n,             // (number) the id of the feature
      "featurename" : "xxxxxxxx",  // (string) the name of the feature
      "activationblock" : n,       // (number) the block the feature will be activated
      "minimumversion" : n         // (number) the minimum client version needed to support this feature
    },
    ...
  ]
  "completedactivations": [    // (array of JSON objects) a list of completed feature activations
    {
      "featureid" : n,             // (number) the id of the feature
      "featurename" : "xxxxxxxx",  // (string) the name of the feature
      "activationblock" : n,       // (number) the block the feature will be activated
      "minimumversion" : n         // (number) the minimum client version needed to support this feature
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_getactivations"
```

---

### counos_getpayload

Get the payload for an Counos transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `txid`              | string  | required | the hash of the transaction to retrieve payload                                              |

**Result:**
```js
{
  "payload" : "payloadmessage",  // (string) the decoded Counos payload message
  "payloadsize" : n              // (number) the size of the payload
}
```

**Example:**

```bash
$ counoscore-cli "counos_getpayload" "1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d"
```

---

### counos_getseedblocks

Returns a list of blocks containing Counos transactions for use in seed block filtering.

WARNING: The Exodus crowdsale is not stored in LevelDB, thus this is currently only safe to use to generate seed blocks after block 255365.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `startblock`        | number  | required | the first block to look for Counos transactions (inclusive)                                    |
| `endblock`          | number  | required | the last block to look for Counos transactions (inclusive)                                     |

**Result:**
```js
[         // (array of numbers) a list of seed blocks
  nnnnnnn,  // the block height of the seed block
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getseedblocks" 290000 300000
```

---

### counos_getcurrentconsensushash

Returns the consensus hash covering the state of the current block.

**Arguments:**

*None*

**Result:**
```js
{
  "block" : nnnnnn,         // (number) the index of the block this consensus hash applies to
  "blockhash" : "hash",     // (string) the hash of the corresponding block
  "consensushash" : "hash"  // (string) the consensus hash for the block
}
```

**Example:**

```bash
$ counoscore-cli "counos_getcurrentconsensushash"
```

---

### omni_getnonfungibletokens

Returns the non-fungible tokens for a given address. Optional property ID filter.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | required | the address                                                                                  |
| `propertyid`        | number  | optional | the property identifier                                                                      |

**Result:**
```js
[
  {
    "propertyid" : n,
    "tokens" : [
      {
        "tokenstart" : n,            // (number) the first token in this range
        "tokenend" : n,              // (number) the last token in this range
        "amount" : n,                // (number) the amount of tokens in the range
      }...
    ]
  }...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getnonfungibletokens 1EXoDusjGwvnjZUyKkxZ4UHEf77z6A5S4P 1"
```

---

### counos_getnonfungibletokendata

Returns owner and all data set in a non-fungible token.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the property identifier                                                                      |
| `tokenidstart`      | number  | required | the first non-fungible token in range                                                        |
| `tokenidend`        | number  | required | the last non-fungible token in range                                                         |

**Result:**
```js
{
  "index" : n,                  // (number) the unique index of the token
  "owner" : "owner",            // (string) the Bitcoin address of the owner
  "grantdata" : "grantdata",    // (string) contents of the grant data field
  "issuerdata" : "issuerdata",  // (string) contents of the issuer data field
  "holderdata" : "holderdata",  // (string) contents of the holder data field
}
```

**Example:**

```bash
$ counoscore-cli "counos_getnonfungibletokendata 1 10 20"
```

---

### counos_getnonfungibletokenranges

Returns the ranges and their addresses for a non-fungible token property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the property identifier                                                                      |

**Result:**
```js
{
  "address" : "address",        // (string) the address
  "tokenstart" : n,            // (number) the first token in this range
  "tokenend" : n,              // (number) the last token in this range
  "amount" : n.nnnnnnnn,       // (number) the amount of tokens in the range
}
```

**Example:**

```bash
$ counoscore-cli "counos_getnonfungibletokenranges 1"
```

---

## Data retrieval (address index)

The following RPCs can be used to obtain information about non-wallet balances and transactions. The address index must be enabled to use them.

### getaddresstxids

Returns the txids for one or more addresses (requires addressindex to be enabled).

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `addresses`         | object  | required | an object with addresses and optional start and end blocks                                   |

**Result:**
```js
[          // (array of txids) a list of transaction hashes
  "hash",  // (string) the transaction identifier
  ...
]
```

**Example:**

```bash
$ counoscore-cli getaddresstxids '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"]}'
```
```bash
$ counoscore-cli getaddresstxids '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"], "start": 380, "end": 400}'
```

---

### getaddressdeltas

Returns all changes for an address (requires addressindex to be enabled).

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `addresses`         | object  | required | an object with addresses and optional start and end blocks                                   |

**Result:**
```js
[
  {
    "satoshis": n,              // (number) the difference of satoshis
    "txid": "hash",             // (string) the related txid
    "index": n,                 // (number) the related input or output index
    "height": n,                // (number) the block height
    "address": "base58address", // (string) the address
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli getaddressdeltas '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"]}'
```
```bash
$ counoscore-cli getaddressdeltas '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"], "start": 380, "end": 400, "chainInfo": false}'
```

---

### getaddressbalance

Returns the balance for one or more addresses (requires addressindex to be enabled).

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `addresses`         | object  | required | an array of addresses                                                                        |

**Result:**
```js
{
  "balance": n,   // (number) the current balance in satoshis
  "received": n,  // (number) the total number of satoshis received (including change)
  "immature": n   // (number) the total number of non-spendable mining satoshis received
}
```

**Example:**

```bash
$ counoscore-cli getaddressbalance '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"]}'
```
```bash
$ counoscore-cli getaddressbalance '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj", "2N2Hca6QvczCnSJ1ZkFfjqDvmgPiWifFF8Q"]}'
```

---

### getaddressutxos

Returns all unspent outputs for an address (requires addressindex to be enabled).

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `addresses`         | object  | required | an array of addresses                                                                        |

**Result:**
```js
{
  "utxos": [
    {
      "address": "base58address", // (string) The address
      "txid": "hash"              // (string) The output txid
      "outputIndex": n,           // (number) The block height
      "script": "hex",            // (string) The script hex encoded
      "satoshis": n,              // (number) The number of satoshis of the output
      "height": 301,              // (number) The block height
      "coinbase": true            // (boolean) Whether it's a coinbase transaction
    },
    ...
  ],
  "hash": "hash",                 // (string) The current block hash
  "height": n                     // (string) The current block height
}
```

**Example:**

```bash
$ counoscore-cli getaddressbalance '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"]}'
```
```bash
$ counoscore-cli getaddressbalance '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"], "chainInfo": true}'
```

---

### getaddressmempool

Returns all mempool deltas for an address (requires addressindex to be enabled).

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `addresses`         | object  | required | an array of addresses                                                                        |

**Result:**
```js
[
  {
    "address": "base58address", // (string) The address
    "txid": "hash"              // (string) The related txid
    "index": n,                 // (number) The related input or output index
    "satoshis": n,              // (number) The difference of satoshis
    "timestamp": n,             // (number) The time the transaction entered the mempool (seconds)
    "prevtxid": "hash",         // (string) The previous txid (if spending)
    "prevout": n                // (number) The previous transaction output index (if spending)
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli getaddressmempool '{"addresses": ["2NDaa1MvFcpc2CAbFvG5g9dDxzrRyDnKnsj"]}'
```

---

### getblockhashes

Returns array of hashes of blocks within the timestamp range provided.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `high`              | number  | required | the newer block timestamp                                                                    |
| `low`               | number  | required | the older block timestamp                                                                    |
| `options`           | object  | optional | an object with options                                                                       |

**Result:**
```js
[
  "hash",               // (string) The block hash
  ...
]
```
```js
[
  {
    "blockhash": "hash", // (string) The block hash
    "logicalts": n       // (number) The logical timestamp
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli getblockhashes 1902163557 1602163557
```
```bash
$ counoscore-cli getblockhashes 1902163557 1602163557 '{"noOrphans":false, "logicalTimes":true}'
```

---

### getspentinfo

Returns the txid and index where an output is spent.

Returns array of hashes of blocks within the timestamp range provided.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `data`              | object  | required | Transaction data                                                                             |

**Result:**
```js
{
  "txid": "hash", // (string) The transaction id
  "index": n,     // (number) The spending input index
  "height": n     // (number) The block height
}
```

**Example:**

```bash
getspentinfo '{"txid": "f5ea8842e96933cbb4d6f07c6dd33902bf3abb2f46cd84ff681ecd288214ea72", "index": 0}'
```

---


## Raw transactions

The RPCs for raw transactions/payloads can be used to decode or create raw Counos transactions.

Raw transactions need to be signed with `"signrawtransaction"` and then broadcasted with `"sendrawtransaction"`.

### counos_decodetransaction

Decodes an Counos transaction.

If the inputs of the transaction are not in the chain, then they must be provided, because the transaction inputs are used to identify the sender of a transaction.

A block height can be provided, which is used to determine the parsing rules.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to decode                                                                |
| `prevtxs`           | string  | optional | a JSON array of transaction inputs (default: none)                                           |
| `height`            | number  | optional | the parsing block height (default: 0 for chain height)                                       |

The format of `prevtxs` is as following:

```js
[
  {
    "txid" : "hash",         // (string, required) the transaction hash
    "vout" : n,              // (number, required) the output number
    "scriptPubKey" : "hex",  // (string, required) the output script
    "value" : n.nnnnnnnn     // (number, required) the output value
  }
  ,...
]
```

**Result:**
```js
{
  "txid" : "hash",                 // (string) the hex-encoded hash of the transaction
  "fee" : "n.nnnnnnnn",            // (string) the transaction fee in counoshs
  "sendingaddress" : "address",    // (string) the CounosH address of the sender
  "referenceaddress" : "address",  // (string) a CounosH address used as reference (if any)
  "ismine" : true|false,           // (boolean) whether the transaction involes an address in the wallet
  "version" : n,                   // (number) the transaction version
  "type_int" : n,                  // (number) the transaction type as number
  "type" : "type",                 // (string) the transaction type as string
  [...]                            // (mixed) other transaction type specific properties
}
```

**Example:**

```bash
$ counoscore-cli "counos_decodetransaction" "010000000163af14ce6d477e1c793507e32a5b7696288fa89705c0d02a3f66beb3c \
    5b8afee0100000000ffffffff02ac020000000000004751210261ea979f6a06f9dafe00fb1263ea0aca959875a7073556a088cdf \
    adcd494b3752102a3fd0a8a067e06941e066f78d930bfc47746f097fcd3f7ab27db8ddf37168b6b52ae22020000000000001976a \
    914946cb2e08075bcbaf157e47bcb67eb2b2339d24288ac00000000" \
    "[{\"txid\":\"eeafb8c5b3be663f2ad0c00597a88f2896765b2ae30735791c7e476dce14af63\",\"vout\":1, \
    \"scriptPubKey\":\"76a9149084c0bd89289bc025d0264f7f23148fb683d56c88ac\",\"value\":0.0001123}]"
```

---

### counos_createrawtx_opreturn

Adds a payload with class C (op-return) encoding to the transaction.

If no raw transaction is provided, a new transaction is created.

If the data encoding fails, then the transaction is not modified.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to extend (can be `null`)                                                |
| `payload`           | string  | required | the hex-encoded payload to add                                                               |

**Result:**
```js
"rawtx"  // (string) the hex-encoded modified raw transaction
```

**Example:**

```bash
$ counoscore-cli "counos_createrawtx_opreturn" "01000000000000000000" "00000000000000020000000006dac2c0"
```

---

### counos_createrawtx_multisig

Adds a payload with class B (bare-multisig) encoding to the transaction.

If no raw transaction is provided, a new transaction is created.

If the data encoding fails, then the transaction is not modified.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to extend (can be `null`)                                                |
| `payload`           | string  | required | the hex-encoded payload to add                                                               |
| `seed`              | string  | required | the seed for obfuscation                                                                     |
| `payload`           | string  | required | a public key or address for dust redemption                                                  |

**Result:**
```js
"rawtx"  // (string) the hex-encoded modified raw transaction
```

**Example:**

```bash
$ counoscore-cli "counos_createrawtx_multisig" \
    "0100000001a7a9402ecd77f3c9f745793c9ec805bfa2e14b89877581c734c774864247e6f50400000000ffffffff01aa0a00000 \
    00000001976a9146d18edfe073d53f84dd491dae1379f8fb0dfe5d488ac00000000" \
    "00000000000000020000000000989680"
    "1LifmeXYHeUe2qdKWBGVwfbUCMMrwYtoMm" \
    "0252ce4bdd3ce38b4ebbc5a6e1343608230da508ff12d23d85b58c964204c4cef3"
```

---

### counos_createrawtx_input

Adds a transaction input to the transaction.

If no raw transaction is provided, a new transaction is created.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to extend (can be `null`)                                                |
| `txid`              | string  | required | the hash of the input transaction                                                            |
| `n`                 | number  | required | the index of the transaction output used as input                                            |

**Result:**
```js
"rawtx"  // (string) the hex-encoded modified raw transaction
```

**Example:**

```bash
$ counoscore-cli "counos_createrawtx_input" \
    "01000000000000000000" "b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee" 0
```

---

### counos_createrawtx_reference

Adds a reference output to the transaction.

If no raw transaction is provided, a new transaction is created.

The output value is set to at least the dust threshold.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to extend (can be `null`)                                                |
| `destination`       | string  | required | the reference address or destination                                                         |
| `referenceamount`   | number  | optional | the optional reference amount (minimal by default)                                           |

**Result:**
```js
"rawtx"  // (string) the hex-encoded modified raw transaction
```

**Example:**

```bash
$ counoscore-cli "counos_createrawtx_reference" \
    "0100000001a7a9402ecd77f3c9f745793c9ec805bfa2e14b89877581c734c774864247e6f50400000000ffffffff03aa0a00000
    00000001976a9146d18edfe073d53f84dd491dae1379f8fb0dfe5d488ac5c0d0000000000004751210252ce4bdd3ce38b4ebbc5a
    6e1343608230da508ff12d23d85b58c964204c4cef3210294cc195fc096f87d0f813a337ae7e5f961b1c8a18f1f8604a909b3a51
    21f065b52aeaa0a000000000000190014d225141d95166c90172443893951321977cb499d00000000" \
    "1CE8bBr1dYZRMnpmyYsFEoexa1YoPz2mfB" \
    0.005
```

---

### counos_createrawtx_change

Adds a change output to the transaction.

The provided inputs are not added to the transaction, but only used to determine the change. It is assumed that the inputs were previously added, for example via `"createrawtransaction"`.

Optionally a position can be provided, where the change output should be inserted, starting with `0`. If the number of outputs is smaller than the position, then the change output is added to the end. Change outputs should be inserted before reference outputs, and as per default, the change output is added to the`first position.

If the change amount would be considered as dust, then no change output is added.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `rawtx`             | string  | required | the raw transaction to extend                                                                |
| `prevtxs`           | string  | required | a JSON array of transaction inputs                                                           |
| `destination`       | string  | required | the destination for the change                                                               |
| `fee`               | number  | required | the desired transaction fees                                                                 |
| `position`          | number  | optional | the position of the change output (default: first position)                                  |

The format of `prevtxs` is as following:

```js
[
  {
    "txid" : "hash",         // (string, required) the transaction hash
    "vout" : n,              // (number, required) the output number
    "scriptPubKey" : "hex",  // (string, required) the output script
    "value" : n.nnnnnnnn     // (number, required) the output value
  }
  ,...
]
```

**Result:**
```js
"rawtx"  // (string) the hex-encoded modified raw transaction
```

**Example:**

```bash
$ counoscore-cli "counos_createrawtx_change" \
    "0100000001b15ee60431ef57ec682790dec5a3c0d83a0c360633ea8308fbf6d5fc10a779670400000000ffffffff025c0d00000 \
    000000047512102f3e471222bb57a7d416c82bf81c627bfcd2bdc47f36e763ae69935bba4601ece21021580b888ff56feb27f17f \
    08802ebed26258c23697d6a462d43fc13b565fda2dd52aeaa0a0000000000001976a914946cb2e08075bcbaf157e47bcb67eb2b2 \
    339d24288ac00000000" \
    "[{\"txid\":\"6779a710fcd5f6fb0883ea3306360c3ad8c0a3c5de902768ec57ef3104e65eb1\",\"vout\":4, \
    \"scriptPubKey\":\"76a9147b25205fd98d462880a3e5b0541235831ae959e588ac\",\"value\":0.00068257}]" \
    "1CE8bBr1dYZRMnpmyYsFEoexa1YoPz2mfB" 0.000035 1
```

---

### counos_createpayload_simplesend

Create the payload for a simple send transaction.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `amount`            | string  | required | the amount to send                                                                           |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_simplesend" 1 "100.0"
```

---

### counos_createpayload_sendall

Create the payload for a send all transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `ecosystem`         | number  | required | the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)             |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_sendall" 2
```

---

### counos_createpayload_dexsell

Create a payload to place, update or cancel a sell offer on the traditional distributed COUNOS/CCH exchange.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale (must be 1 for COUN or 2 for TCOUN)              |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `amountdesired`     | string  | required | the amount of counoshs desired                                                               |
| `paymentwindow`     | number  | required | a time limit in blocks a buyer has to pay following a successful accepting order             |
| `minacceptfee`      | string  | required | a minimum mining fee a buyer has to pay to accept the offer                                  |
| `action`            | number  | required | the action to take (1 for new offers, 2 to update\", 3 to cancel)                            |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_dexsell" 1 "1.5" "0.75" 25 "0.0005" 1
```

---

### counos_createpayload_dexaccept

Create the payload for an accept offer for the specified token and amount.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the token to purchase                                                      |
| `amount`            | string  | required | the amount to accept                                                                         |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_dexaccept" 1 "15.0"
```

---

### counos_createpayload_sto

Creates the payload for a send-to-owners transaction.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                   | Type    | Presence | Description                                                                                  |
|------------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`           | number  | required | the identifier of the token to distribute                                                    |
| `amount`               | string  | required | the amount to distribute                                                                     |
| `distributionproperty` | number  | optional | the identifier of the property holders to distribute to                                      |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_sto" 3 "5000"
```

---

### counos_createpayload_issuancefixed

Creates the payload for a new tokens issuance with fixed supply.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)           |
| `type`              | number  | required | the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)         |
| `previousid`        | number  | required | an identifier of a predecessor token (use 0 for new tokens)                                  |
| `category`          | string  | required | a category for the new tokens (can be "")                                                    |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be "")                                                 |
| `name`              | string  | required | the name of the new tokens to create                                                         |
| `url`               | string  | required | an URL for further information about the new tokens (can be "")                              |
| `data`              | string  | required | a description for the new tokens (can be "")                                                 |
| `amount`            | string  | required | the number of tokens to create                                                               |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_issuancefixed" 2 1 0 "Companies" "CounosH Mining" "Quantum Miner" "" "" "1000000"
```

---

### counos_createpayload_issuancecrowdsale

Creates the payload for a new tokens issuance with crowdsale.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)           |
| `type`              | number  | required | the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)         |
| `previousid`        | number  | required | an identifier of a predecessor token (use 0 for new tokens)                                  |
| `category`          | string  | required | a category for the new tokens (can be "")                                                    |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be "")                                                 |
| `name`              | string  | required | the name of the new tokens to create                                                         |
| `url`               | string  | required | an URL for further information about the new tokens (can be "")                              |
| `data`              | string  | required | a description for the new tokens (can be "")                                                 |
| `propertyiddesired` | number  | required | the identifier of a token eligible to participate in the crowdsale                           |
| `tokensperunit`     | string  | required | the amount of tokens granted per unit invested in the crowdsale                              |
| `deadline`          | number  | required | the deadline of the crowdsale as Unix timestamp                                              |
| `earlybonus`        | number  | required | an early bird bonus for participants in percent per week                                     |
| `issuerpercentage`  | number  | required | a percentage of tokens that will be granted to the issuer                                    |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_issuancecrowdsale" 2 1 0 "Companies" "CounosH Mining" "Quantum Miner" "" "" 2 "100" 1483228800 30 2
```

---

### counos_createpayload_issuancemanaged

Creates the payload for a new tokens issuance with manageable supply.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `ecosystem`         | number  | required | the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)           |
| `type`              | number  | required | the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)         |
| `previousid`        | number  | required | an identifier of a predecessor token (use 0 for new tokens)                                  |
| `category`          | string  | required | a category for the new tokens (can be "")                                                    |
| `subcategory`       | string  | required | a subcategory for the new tokens (can be "")                                                 |
| `name`              | string  | required | the name of the new tokens to create                                                         |
| `url`               | string  | required | an URL for further information about the new tokens (can be "")                              |
| `data`              | string  | required | a description for the new tokens (can be "")                                                 |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_issuancemanaged" 2 1 0 "Companies" "CounosH Mining" "Quantum Miner" "" ""
```

---

### counos_createpayload_closecrowdsale

Creates the payload to manually close a crowdsale.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the crowdsale to close                                                     |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_closecrowdsale" 70
```

---

### counos_createpayload_grant

Creates the payload to issue or grant new units of managed tokens.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name         | Type    | Presence | Description                                                           |
|--------------|---------|----------|-----------------------------------------------------------------------|
| `propertyid` | number  | required | the identifier of the tokens to grant                                 |
| `amount`     | string  | required | the amount of tokens to create                                        |
| `grantdata`  | string  | optional | NFT only: data set in all NFTs created in this grant (default: empty) |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_grant" 51 "7000"
```

---

### counos_createpayload_revoke

Creates the payload to revoke units of managed tokens.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!f

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens to revoke                                                       |
| `amount`            | string  | required | the amount of tokens to revoke                                                               |
| `memo`              | string  | optional | a text note attached to this transaction (none by default)                                   |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_revoke" 51 "100"
```

---

### counos_createpayload_changeissuer

Creates the payload to change the issuer on record of the given tokens.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_changeissuer" 3
```

---

### counos_createpayload_trade

Creates the payload to place a trade offer on the distributed token exchange.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `propertyiddesired` | number  | required | the identifier of the tokens desired in exchange                                             |
| `amountdesired`     | string  | required | the amount of tokens desired in exchange                                                     |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_trade" 31 "250.0" 1 "10.0"
```

---

### counos_createpayload_canceltradesbyprice

Creates the payload to cancel offers on the distributed token exchange with the specified price.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `amountforsale`     | string  | required | the amount of tokens to list for sale                                                        |
| `propertyiddesired` | number  | required | the identifier of the tokens desired in exchange                                             |
| `amountdesired`     | string  | required | the amount of tokens desired in exchange                                                     |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_canceltradesbyprice" 31 "100.0" 1 "5.0"
```

---

### counos_createpayload_canceltradesbypair

Creates the payload to cancel all offers on the distributed token exchange with the given currency pair.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyidforsale` | number  | required | the identifier of the tokens to list for sale                                                |
| `propertyiddesired` | number  | required | the identifier of the tokens desired in exchange                                             |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_canceltradesbypair" 1 31
```

---

### counos_createpayload_cancelalltrades

Creates the payload to cancel all offers on the distributed token exchange with the given currency pair.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `ecosystem`         | number  | required | the ecosystem of the offers to cancel (1 for main ecosystem, 2 for test ecosystem)           |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_cancelalltrades" 1
```

---

### counos_createpayload_enablefreezing

Creates the payload to enable address freezing for a centrally managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_enablefreezing" 3
```

---

### counos_createpayload_disablefreezing

Creates the payload to disable address freezing for a centrally managed property.

IMPORTANT NOTE:  Disabling freezing for a property will UNFREEZE all frozen addresses for that property!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens                                                                 |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_disablefreezing" 3
```

---

### counos_createpayload_freeze

Creates the payload to freeze an address for a centrally managed token.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                                 |
|---------------------|---------|----------|-------------------------------------------------------------------------------------------------------------|
| `toaddress`         | string  | required | the address to freeze tokens for                                                                            |
| `propertyid`        | number  | required | the property to freeze tokens for (must be managed type and have freezing option enabled)                   |
| `amount`            | string  | required | the amount of tokens to freeze (note: this is unused - once frozen an address cannot send any transactions) |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_freeze" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs" 31 "100"
```

---

### counos_createpayload_unfreeze

Creates the payload to unfreeze an address for a centrally managed token.

Note: if the server is not synchronized, amounts are considered as divisible, even if the token may have indivisible units!

**Arguments:**

| Name                | Type    | Presence | Description                                                                                                 |
|---------------------|---------|----------|-------------------------------------------------------------------------------------------------------------|
| `toaddress`         | string  | required | the address to unfreeze tokens for                                                                          |
| `propertyid`        | number  | required | the property to unfreeze tokens for (must be managed type and have freezing option enabled)                 |
| `amount`            | string  | required | the amount of tokens to unfreeze (note: this is unused)                                                     |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_unfreeze" "3HTHRxu3aSDV4deakjC7VmsiUp7c6dfbvs" 31 "100"
```

---

### omni_createpayload_adddelegate

Creates the payload to add a delegate for the issuance of tokens of a managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                                 |
|---------------------|---------|----------|-------------------------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens                                                                                |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ omnicore-cli "omni_createpayload_adddelegate" 21
```

---

### omni_createpayload_removedelegate

Creates the payload to remove a delegate for the issuance of tokens of a managed property.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                                 |
|---------------------|---------|----------|-------------------------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens                                                                                |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_removedelegate" 21


---

### counos_createpayload_anydata

Creates the payload to embed arbitrary data.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                                 |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `data`              | string  | required | the hex-encoded data                                                                         |

**Result:**
```js
"payload"  // (string) the hex-encoded payload
```

**Example:**

```bash
$ counoscore-cli "counos_createpayload_anydata" "646578782032303230"
```

---

### counos_createpayload_sendnonfungible
Create the payload for a non-fungible send transaction.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `tokenstart`        | number  | required | the first token in the range to send                                                         |
| `tokenend`          | number  | required | the last token in the range to send                                                          |
**Result:**
```js
"payload"  // (string) the hex-encoded payload
```
**Example:**
```bash
$ counoscore-cli "counos_createpayload_sendnonfungible" 70 1 1000
```
---
### counos_createpayload_setnonfungibledata
Create the payload for a non-fungible token set data transaction.
**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the tokens to send                                                         |
| `tokenstart`        | number  | required | the first token in the range to set data on                                                  |
| `tokenend`          | number  | required | the last token in the range to set data on                                                   |
| `issuer`            | boolean | required | if true issuer data set, otherwise holder data set                                           |
| `data`              | string  | required | data set as in either issuer or holder fields                                                |
**Result:**
```js
"payload"  // (string) the hex-encoded payload
```
**Example:**
```bash
$ counoscore-cli "counos_createpayload_setnonfungibledata" 70 50 60 true "string data"
```
---

## Fee system

The RPCs for the fee system can be used to obtain data about the fee system and fee distributions.

### counos_getfeecache

Obtains the current amount of fees cached (pending distribution).

If a property ID is supplied the results will be filtered to show this property ID only.  If no property ID is supplied the results will contain all properties that currently have fees cached pending distribution.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | optional | the identifier of the property to filter results on                                          |

**Result:**
```js
[                                  // (array of JSON objects)
  {
    "propertyid" : nnnnnnn,        // (number) the property id
    "cachedfees" : "n.nnnnnnnn",   // (string) the amount of fees cached for this property
  },
...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getfeecache" 31
```

---

### counos_getfeetrigger

Obtains the amount at which cached fees will be distributed.

If a property ID is supplied the results will be filtered to show this property ID only.  If no property ID is supplied the results will contain all properties.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | optional | the identifier of the property to filter results on                                          |

**Result:**
```js
[                                  // (array of JSON objects)
  {
    "propertyid" : nnnnnnn,        // (number) the property id
    "feetrigger" : "n.nnnnnnnn",   // (string) the amount of fees required to trigger distribution
  },
...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getfeetrigger" 31
```

---

### counos_getfeeshare

Obtains the current percentage share of fees addresses would receive if a distribution were to occur.

If an address is supplied the results will be filtered to show this address only.  If no address is supplied the results will be filtered to show wallet addresses only.  If a wildcard is provided (```"*"```) the results will contain all addresses that would receive a share.

If an ecosystem is supplied the results will reflect the fee share for that ecosystem (main or test).  If no ecosystem is supplied the results will reflect the main ecosystem.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `address`           | string  | optional | the address to filter results on                                                             |
| `ecosystem`         | number  | optional | the ecosystem to obtain the current percentage fee share (1 = main, 2 = test)                |

**Result:**
```js
[                                  // (array of JSON objects)
  {
    "address" : "address"          // (string) the address that would receive a share of fees
    "feeshare" : "n.nnnn%",        // (string) the percentage of fees this address will receive based on the current state
  },
...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getfeeshare" "1CE8bBr1dYZRMnpmyYsFEoexa1YoPz2mfB" 1
```

---

### counos_getfeedistribution

Obtains data for a past distribution of fees.

A distribution ID must be supplied to identify the distribution to obtain data for.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `distributionid`    | number  | required | the identifier of the distribution to obtain data for                                        |

**Result:**
```js
{
  "distributionid" : n,            // (number) the distribution id
  "propertyid" : n,                // (number) the property id of the distributed tokens
  "block" : n,                     // (number) the block the distribution occurred
  "amount" : "n.nnnnnnnn",         // (string) the amount that was distributed
  "recipients": [                  // (array of JSON objects) a list of recipients
    {
      "address" : "address",       // (string) the address of the recipient
      "amount" : "n.nnnnnnnn"      // (string) the amount of fees received by the recipient
    },
    ...
  ]
}
```

**Example:**

```bash
$ counoscore-cli "counos_getfeedistribution" 1
```

---

### counos_getfeedistributions

Obtains data for past distributions of fees for a property.

A property ID must be supplied to retrieve past distributions for.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `propertyid`        | number  | required | the identifier of the property to retrieve past distributions for                            |

**Result:**
```js
[                                  // (array of JSON objects)
  {
    "distributionid" : n,          // (number) the distribution id
    "propertyid" : n,              // (number) the property id of the distributed tokens
    "block" : n,                   // (number) the block the distribution occurred
    "amount" : "n.nnnnnnnn",       // (string) the amount that was distributed
    "recipients": [                // (array of JSON objects) a list of recipients
      {
        "address" : "address",       // (string) the address of the recipient
        "amount" : "n.nnnnnnnn"      // (string) the amount of fees received by the recipient
      },
      ...
    ]
  },
  ...
]
```

**Example:**

```bash
$ counoscore-cli "counos_getfeedistributions" 31
```

---

## Configuration

The RPCs for the configuration can be used to alter Counos Core settings.

### counos_setautocommit

Sets the global flag that determines whether transactions are automatically committed and broadcasted.

**Arguments:**

| Name                | Type    | Presence | Description                                                                                  |
|---------------------|---------|----------|----------------------------------------------------------------------------------------------|
| `flag`              | boolean | required | the flag                                                                                     |

**Result:**
```js
true|false  // (boolean) the updated flag status
```

**Example:**

```bash
$ counoscore-cli "counos_setautocommit" false
```

---

## Deprecated API calls

To ensure backwards compatibility, deprecated RPCs are kept for at least one major version.

The following calls are replaced in Counos Core 0.0.10, and queries with the old command are forwarded.

- `send_MP` by `counos_send`
- `sendtoowners_MP` by `counos_sendsto`
- `sendrawtx_MP` by `counos_sendrawtx`
- `getinfo_MP` by `counos_getinfo`
- `getbalance_MP` by `counos_getbalance`
- `getallbalancesforid_MP` by `counos_getallbalancesforid`
- `getallbalancesforaddress_MP` by `counos_getallbalancesforaddress`
- `gettransaction_MP` by `counos_gettransaction`
- `listtransactions_MP` by `counos_listtransactions`
- `listblocktransactions_MP` by `counos_listblocktransactions`
- `getactivedexsells_MP` by `counos_getactivedexsells`
- `listproperties_MP` by `counos_listproperties`
- `getproperty_MP` by `counos_getproperty`
- `getactivecrowdsales_MP` by `counos_getactivecrowdsales`
- `getcrowdsale_MP` by `counos_getcrowdsale`
- `getgrants_MP` by `counos_getgrants`
- `getsto_MP` by `counos_getsto` or `counos_gettransaction`
