/**
 * @file rpctx.cpp
 *
 * This file contains RPC calls for creating and sending Counos transactions.
 */

#include <counoscore/createpayload.h>
#include <counoscore/dex.h>
#include <counoscore/errors.h>
#include <counoscore/counoscore.h>
#include <counoscore/pending.h>
#include <counoscore/rpcrequirements.h>
#include <counoscore/rpcvalues.h>
#include <counoscore/rules.h>
#include <counoscore/sp.h>
#include <counoscore/tx.h>
#include <counoscore/utilscounosh.h>
#include <counoscore/wallettxbuilder.h>

#include <interfaces/wallet.h>
#include <init.h>
#include <key_io.h>
#include <validation.h>
#include <wallet/rpcwallet.h>
#include <rpc/blockchain.h>
#include <rpc/server.h>
#include <rpc/util.h>
#include <sync.h>
#include <util/moneystr.h>
#include <wallet/wallet.h>

#include <univalue.h>

#include <stdint.h>
#include <stdexcept>
#include <string>

using std::runtime_error;
using namespace mastercore;


static UniValue counos_funded_send(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_funded_send",
       "\nCreates and sends a funded simple send transaction.\n"
       "\nAll counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send the tokens from"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the receiver"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to send"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount to send"},
           {"feeaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address that is used for change and to pay for fees, if needed"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_funded_send", "\"1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH\" \"15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH\" 1 \"100.0\" \"15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1\"")
           + HelpExampleRpc("counos_funded_send", "\"1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH\", \"15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH\", 1, \"100.0\", \"15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));
    std::string feeAddress = ParseAddress(request.params[4]);

    // perform checks
    RequireExistingProperty(propertyId);
    RequireBalance(fromAddress, propertyId, amount);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_SimpleSend(propertyId, amount);

    // create the raw transaction
    uint256 retTxid;
    int result = CreateFundedTransaction(fromAddress, toAddress, feeAddress, payload, retTxid, pwallet.get(), *g_rpc_node);
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    }

    return retTxid.ToString();
}

static UniValue counos_funded_sendall(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_funded_sendall",
       "\nCreates and sends a transaction that transfers all available tokens in the given ecosystem to the recipient.\n"
       "\nAll counoshs from the sender are consumed and if there are counoshs missing, they are taken from the specified fee source. Change is sent to the fee source!\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to the tokens send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the receiver\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)\n"},
           {"feeaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address that is used for change and to pay for fees, if needed\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_funded_sendall", "\"1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH\" \"15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH\" 1 \"15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1\"")
           + HelpExampleRpc("counos_funded_sendall", "\"1DFa5bT6KMEr6ta29QJouainsjaNBsJQhH\", \"15cWrfuvMxyxGst2FisrQcvcpF48x6sXoH\", 1, \"15Jhzz4omEXEyFKbdcccJwuVPea5LqsKM1\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint8_t ecosystem = ParseEcosystem(request.params[2]);
    std::string feeAddress = ParseAddress(request.params[3]);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_SendAll(ecosystem);

    // create the raw transaction
    uint256 retTxid;
    int result = CreateFundedTransaction(fromAddress, toAddress, feeAddress, payload, retTxid, pwallet.get(), *g_rpc_node);
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    }

    return retTxid.ToString();
}

static UniValue counos_sendrawtx(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendrawtx",
       "\nBroadcasts a raw Counos Layer transaction.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"rawtransaction", RPCArg::Type::STR, RPCArg::Optional::NO, "the hex-encoded raw transaction"},
           {"referenceaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a reference address (none by default)\n"},
           {"redeemaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "an address that can spent the transaction dust (sender by default)\n"},
           {"referenceamount", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a counosh amount that is sent to the receiver (minimal by default)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendrawtx", "\"1MCHESTptvd2LnNp7wmr2sGTpRomteAkq8\" \"000000000000000100000000017d7840\" \"1EqTta1Rt8ixAA32DuC29oukbsSWU62qAV\"")
           + HelpExampleRpc("counos_sendrawtx", "\"1MCHESTptvd2LnNp7wmr2sGTpRomteAkq8\", \"000000000000000100000000017d7840\", \"1EqTta1Rt8ixAA32DuC29oukbsSWU62qAV\"")
       }
    }.Check(request);

    std::string fromAddress = ParseAddress(request.params[0]);
    std::vector<unsigned char> data = ParseHexV(request.params[1], "raw transaction");
    std::string toAddress = (request.params.size() > 2) ? ParseAddressOrEmpty(request.params[2]): "";
    std::string redeemAddress = (request.params.size() > 3) ? ParseAddressOrEmpty(request.params[3]): "";
    int64_t referenceAmount = (request.params.size() > 4) ? ParseAmount(request.params[4], true): 0;

    //some sanity checking of the data supplied?
    uint256 newTX;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, redeemAddress, referenceAmount, data, newTX, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return newTX.GetHex();
        }
    }
}

static UniValue counos_send(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_send",
       "\nCreate and broadcast a simple send transaction.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the receiver\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to send\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount to send\n"},
           {"redeemaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "an address that can spend the transaction dust (sender by default)\n"},
           {"referenceamount", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a counosh amount that is sent to the receiver (minimal by default)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_send", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\" \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"100.0\"")
           + HelpExampleRpc("counos_send", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\", \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"100.0\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));
    std::string redeemAddress = (request.params.size() > 4 && !ParseText(request.params[4]).empty()) ? ParseAddress(request.params[4]): "";
    int64_t referenceAmount = (request.params.size() > 5) ? ParseAmount(request.params[5], true): 0;

    // perform checks
    RequireExistingProperty(propertyId);
    RequireBalance(fromAddress, propertyId, amount);
    RequireSaneReferenceAmount(referenceAmount);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_SimpleSend(propertyId, amount);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, redeemAddress, referenceAmount, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_SIMPLE_SEND, propertyId, amount);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendall(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendall",
       "\nTransfers all available tokens in the given ecosystem to the recipient.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the receiver\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem of the tokens to send (1 for main ecosystem, 2 for test ecosystem)\n"},
           {"redeemaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "an address that can spend the transaction dust (sender by default)\n"},
           {"referenceamount", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a counosh amount that is sent to the receiver (minimal by default)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendall", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\" \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 2")
           + HelpExampleRpc("counos_sendall", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\", \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 2")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint8_t ecosystem = ParseEcosystem(request.params[2]);
    std::string redeemAddress = (request.params.size() > 3 && !ParseText(request.params[3]).empty()) ? ParseAddress(request.params[3]): "";
    int64_t referenceAmount = (request.params.size() > 4) ? ParseAmount(request.params[4], true): 0;

    // perform checks
    RequireSaneReferenceAmount(referenceAmount);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_SendAll(ecosystem);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, redeemAddress, referenceAmount, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            // TODO: pending
            return txid.GetHex();
        }
    }
}

static UniValue counos_senddexsell(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_senddexsell",
       "\nPlace, update or cancel a sell offer on the distributed token/CCH exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to list for sale\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to list for sale\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of counoshs desired\n"},
           {"paymentwindow", RPCArg::Type::NUM, RPCArg::Optional::NO, "a time limit in blocks a buyer has to pay following a successful accepting order\n"},
           {"minacceptfee", RPCArg::Type::STR, RPCArg::Optional::NO, "a minimum mining fee a buyer has to pay to accept the offer\n"},
           {"action", RPCArg::Type::NUM, RPCArg::Optional::NO, "the action to take (1 for new offers, 2 to update\", 3 to cancel)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_senddexsell", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"1.5\" \"0.75\" 25 \"0.0001\" 1")
           + HelpExampleRpc("counos_senddexsell", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"1.5\", \"0.75\", 25, \"0.0001\", 1")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = 0; // depending on action
    int64_t amountDesired = 0; // depending on action
    uint8_t paymentWindow = 0; // depending on action
    int64_t minAcceptFee = 0;  // depending on action
    uint8_t action = ParseDExAction(request.params[6]);

    // perform conversions
    if (action <= CMPTransaction::UPDATE) { // actions 3 permit zero values, skip check
        amountForSale = ParseAmount(request.params[2], isPropertyDivisible(propertyIdForSale));
        amountDesired = ParseAmount(request.params[3], true); // CCH is divisible
        paymentWindow = ParseDExPaymentWindow(request.params[4]);
        minAcceptFee = ParseDExFee(request.params[5]);
    }

    // perform checks
    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyIdForSale);
    }

    switch (action) {
        case CMPTransaction::NEW:
        {
            RequireBalance(fromAddress, propertyIdForSale, amountForSale);
            RequireNoOtherDExOffer(fromAddress);
            break;
        }
        case CMPTransaction::UPDATE:
        {
            RequireBalance(fromAddress, propertyIdForSale, amountForSale);
            RequireMatchingDExOffer(fromAddress, propertyIdForSale);
            break;
        }
        case CMPTransaction::CANCEL:
        {
            RequireMatchingDExOffer(fromAddress, propertyIdForSale);
            break;
        }
    }

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DExSell(propertyIdForSale, amountForSale, amountDesired, paymentWindow, minAcceptFee, action);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            bool fSubtract = (action <= CMPTransaction::UPDATE); // no pending balances for cancels
            PendingAdd(txid, fromAddress, MSC_TYPE_TRADE_OFFER, propertyIdForSale, amountForSale, fSubtract);
            return txid.GetHex();
        }
    }
}

static UniValue counos_senddexaccept(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_senddexaccept",
       "\nCreate and broadcast an accept offer for the specified token and amount.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the seller\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the token to purchase\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount to accept\n"},
           {"override", RPCArg::Type::BOOL, RPCArg::Optional::OMITTED, "override minimum accept fee and payment window checks (use with caution!)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_senddexaccept", "\"35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8\" \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"15.0\"")
           + HelpExampleRpc("counos_senddexaccept", "\"35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8\", \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"15.0\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));
    bool override = (request.params.size() > 4) ? request.params[4].get_bool(): false;

    // perform checks
    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyId);
    }
    RequireMatchingDExOffer(toAddress, propertyId);

    if (!override) { // reject unsafe accepts - note client maximum tx fee will always be respected regardless of override here
        RequireSaneDExFee(toAddress, propertyId);
        RequireSaneDExPaymentWindow(toAddress, propertyId);
    }

    int64_t nMinimumAcceptFee = 0;
    // use new 0.10 custom fee to set the accept minimum fee appropriately
    {
        LOCK(cs_tally);
        const CMPOffer* sellOffer = DEx_getOffer(toAddress, propertyId);
        if (sellOffer == nullptr) throw JSONRPCError(RPC_TYPE_ERROR, "Unable to load sell offer from the distributed exchange");
        nMinimumAcceptFee = sellOffer->getMinFee();
    }

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DExAccept(propertyId, amount);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, "", 0, payload, txid, rawHex, autoCommit, pwallet.get(), nMinimumAcceptFee);

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendnewdexorder(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendnewdexorder",
       "\nCreates a new sell offer on the distributed token/CCH exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to list for sale\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to list for sale\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of counoshs desired\n"},
           {"paymentwindow", RPCArg::Type::NUM, RPCArg::Optional::NO, "a time limit in blocks a buyer has to pay following a successful accepting order\n"},
           {"minacceptfee", RPCArg::Type::STR, RPCArg::Optional::NO, "a minimum mining fee a buyer has to pay to accept the offer\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendnewdexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"1.5\" \"0.75\" 50 \"0.0001\"")
           + HelpExampleRpc("counos_sendnewdexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"1.5\", \"0.75\", 50, \"0.0001\"")
       }
    }.Check(request);

    // parse parameters
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = ParseAmount(request.params[2], isPropertyDivisible(propertyIdForSale));
    int64_t amountDesired = ParseAmount(request.params[3], true); // CCH is divisible
    uint8_t paymentWindow = ParseDExPaymentWindow(request.params[4]);
    int64_t minAcceptFee = ParseDExFee(request.params[5]);
    uint8_t action = CMPTransaction::NEW;

    // perform checks
    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyIdForSale);
    }
    RequireBalance(fromAddress, propertyIdForSale, amountForSale);
    RequireNoOtherDExOffer(fromAddress);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DExSell(
        propertyIdForSale,
        amountForSale,
        amountDesired,
        paymentWindow,
        minAcceptFee,
        action);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    }
    if (!autoCommit) {
        return rawHex;
    }

    PendingAdd(txid, fromAddress, MSC_TYPE_TRADE_OFFER, propertyIdForSale, amountForSale, true);
    return txid.GetHex();
}

static UniValue counos_sendupdatedexorder(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendupdatedexorder",
       "\nUpdates an existing sell offer on the distributed token/CCH exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to update\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the new amount of tokens to list for sale\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the new amount of counoshs desired\n"},
           {"paymentwindow", RPCArg::Type::NUM, RPCArg::Optional::NO, "a new time limit in blocks a buyer has to pay following a successful accepting order\n"},
           {"minacceptfee", RPCArg::Type::STR, RPCArg::Optional::NO, "a new minimum mining fee a buyer has to pay to accept the offer\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendupdatedexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"1.0\" \"1.75\" 50 \"0.0001\"")
           + HelpExampleRpc("counos_sendupdatedexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"1.0\", \"1.75\", 50, \"0.0001\"")
       }
    }.Check(request);

    // parse parameters
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = ParseAmount(request.params[2], isPropertyDivisible(propertyIdForSale));
    int64_t amountDesired = ParseAmount(request.params[3], true); // CCH is divisible
    uint8_t paymentWindow = ParseDExPaymentWindow(request.params[4]);
    int64_t minAcceptFee = ParseDExFee(request.params[5]);
    uint8_t action = CMPTransaction::UPDATE;

    // perform checks
    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyIdForSale);
    }
    RequireBalance(fromAddress, propertyIdForSale, amountForSale);
    RequireMatchingDExOffer(fromAddress, propertyIdForSale);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DExSell(
        propertyIdForSale,
        amountForSale,
        amountDesired,
        paymentWindow,
        minAcceptFee,
        action);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    }
    if (!autoCommit) {
        return rawHex;
    }

    PendingAdd(txid, fromAddress, MSC_TYPE_TRADE_OFFER, propertyIdForSale, amountForSale, true);
    return txid.GetHex();
}

static UniValue counos_sendcanceldexorder(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendcanceldexorder",
       "\nCancels existing sell offer on the distributed token/CCH exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to cancel\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendcanceldexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1")
           + HelpExampleRpc("counos_sendcanceldexorder", "\"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1")
       }
    }.Check(request);

    // parse parameters
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = 0;
    int64_t amountDesired = 0;
    uint8_t paymentWindow = 0;
    int64_t minAcceptFee = 0;
    uint8_t action = CMPTransaction::CANCEL;

    // perform checks
    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyIdForSale);
    }
    RequireMatchingDExOffer(fromAddress, propertyIdForSale);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DExSell(
        propertyIdForSale,
        amountForSale,
        amountDesired,
        paymentWindow,
        minAcceptFee,
        action);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    }
    if (!autoCommit) {
        return rawHex;
    }

    PendingAdd(txid, fromAddress, MSC_TYPE_TRADE_OFFER, propertyIdForSale, amountForSale, false);
    return txid.GetHex();
}

static UniValue counos_senddexpay(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_senddexpay",
       "\nCreate and broadcast payment for an accept offer.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address of the seller\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the token to purchase\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the CounosH amount to send\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_senddexaccept", "\"35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8\" \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 1 \"15.0\"")
           + HelpExampleRpc("counos_senddexaccept", "\"35URq1NN3xL6GeRKUP6vzaQVcxoJiiJKd8\", \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 1, \"15.0\"")
       }
    }.Check(request);

    // Parameters
    std::string buyerAddress = ParseText(request.params[0]);
    std::string sellerAddress = ParseText(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    CAmount nAmount = ParseAmount(request.params[3], true);

    // Check parameters are valid
    if (nAmount <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount for send");

    CTxDestination buyerDest = DecodeDestination(buyerAddress);
    if (!IsValidDestination(buyerDest)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid buyer address");
    }

    CTxDestination sellerDest = DecodeDestination(sellerAddress);
    if (!IsValidDestination(sellerDest)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid seller address");
    }

    if (!IsFeatureActivated(FEATURE_FREEDEX, GetHeight())) {
        RequirePrimaryToken(propertyId);
    }
    RequireMatchingDExAccept(sellerAddress, propertyId, buyerAddress);

    // Get accept offer and make sure buyer is not trying to overpay
    {
        LOCK(cs_tally);
        const CMPAccept* acceptOffer = DEx_getAccept(sellerAddress, propertyId, buyerAddress);
        if (acceptOffer == nullptr)
            throw JSONRPCError(RPC_MISC_ERROR, "Unable to load accept offer from the distributed exchange");

        const CAmount amountAccepted = acceptOffer->getAcceptAmountRemaining();
        const CAmount amountToPayInCCH = calculateDesiredCCH(acceptOffer->getOfferAmountOriginal(), acceptOffer->getCCHDesiredOriginal(), amountAccepted);

        if (nAmount > amountToPayInCCH) {
            throw JSONRPCError(RPC_MISC_ERROR, strprintf("Paying more than required: %lld CCH to pay for %lld tokens", FormatMoney(amountToPayInCCH), FormatMP(propertyId, amountAccepted)));
        }

        if (!isPropertyDivisible(propertyId) && nAmount < amountToPayInCCH) {
            throw JSONRPCError(RPC_MISC_ERROR, strprintf("Paying less than required: %lld CCH to pay for %lld tokens", FormatMoney(amountToPayInCCH), FormatMP(propertyId, amountAccepted)));
        }
    }

    uint256 txid;
    int result = CreateDExTransaction(pwallet.get(), buyerAddress, sellerAddress, nAmount, txid);

    // Check error and return the txid
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        return txid.GetHex();
    }
}

static UniValue counos_sendissuancecrowdsale(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendissuancecrowdsale",
       "Create new tokens as crowdsale.",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"},
           {"type", RPCArg::Type::NUM, RPCArg::Optional::NO, "the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"},
           {"previousid", RPCArg::Type::NUM, RPCArg::Optional::NO, "an identifier of a predecessor token (0 for new crowdsales)\n"},
           {"category", RPCArg::Type::STR, RPCArg::Optional::NO, "a category for the new tokens (can be \"\")\n"},
           {"subcategory", RPCArg::Type::STR, RPCArg::Optional::NO, "a subcategory for the new tokens  (can be \"\")\n"},
           {"name", RPCArg::Type::STR, RPCArg::Optional::NO, "the name of the new tokens to create\n"},
           {"url", RPCArg::Type::STR, RPCArg::Optional::NO, "a URL for further information about the new tokens (can be \"\")\n"},
           {"data", RPCArg::Type::STR, RPCArg::Optional::NO, "a description for the new tokens (can be \"\")\n"},
           {"propertyiddesired", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of a token eligible to participate in the crowdsale\n"},
           {"tokensperunit", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens granted per unit invested in the crowdsale\n"},
           {"deadline", RPCArg::Type::NUM, RPCArg::Optional::NO, "the deadline of the crowdsale as Unix timestamp\n"},
           {"earlybonus", RPCArg::Type::NUM, RPCArg::Optional::NO, "an early bird bonus for participants in percent per week\n"},
           {"issuerpercentage", RPCArg::Type::NUM, RPCArg::Optional::NO, "a percentage of tokens that will be granted to the issuer\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendissuancecrowdsale", "\"3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo\" 2 1 0 \"Companies\" \"CounosH Mining\" \"Quantum Miner\" \"\" \"\" 2 \"100\" 1483228800 30 2")
           + HelpExampleRpc("counos_sendissuancecrowdsale", "\"3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo\", 2, 1, 0, \"Companies\", \"CounosH Mining\", \"Quantum Miner\", \"\", \"\", 2, \"100\", 1483228800, 30, 2")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint8_t ecosystem = ParseEcosystem(request.params[1]);
    uint16_t type = ParsePropertyType(request.params[2]);
    uint32_t previousId = ParsePreviousPropertyId(request.params[3]);
    std::string category = ParseText(request.params[4]);
    std::string subcategory = ParseText(request.params[5]);
    std::string name = ParseText(request.params[6]);
    std::string url = ParseText(request.params[7]);
    std::string data = ParseText(request.params[8]);
    uint32_t propertyIdDesired = ParsePropertyId(request.params[9]);
    int64_t numTokens = ParseAmount(request.params[10], type);
    int64_t deadline = ParseDeadline(request.params[11]);
    uint8_t earlyBonus = ParseEarlyBirdBonus(request.params[12]);
    uint8_t issuerPercentage = ParseIssuerBonus(request.params[13]);

    // perform checks
    RequirePropertyName(name);
    RequireExistingProperty(propertyIdDesired);
    RequireSameEcosystem(ecosystem, propertyIdDesired);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_IssuanceVariable(ecosystem, type, previousId, category, subcategory, name, url, data, propertyIdDesired, numTokens, deadline, earlyBonus, issuerPercentage);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendissuancefixed(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendissuancefixed",
       "\nCreate new tokens with fixed supply.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"},
           {"type", RPCArg::Type::NUM, RPCArg::Optional::NO, "the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"},
           {"previousid", RPCArg::Type::NUM, RPCArg::Optional::NO, "an identifier of a predecessor token (use 0 for new tokens)\n"},
           {"category", RPCArg::Type::STR, RPCArg::Optional::NO, "a category for the new tokens (can be \"\")\n"},
           {"subcategory", RPCArg::Type::STR, RPCArg::Optional::NO, "a subcategory for the new tokens  (can be \"\")\n"},
           {"name", RPCArg::Type::STR, RPCArg::Optional::NO, "the name of the new tokens to create\n"},
           {"url", RPCArg::Type::STR, RPCArg::Optional::NO, "a URL for further information about the new tokens (can be \"\")\n"},
           {"data", RPCArg::Type::STR, RPCArg::Optional::NO, "a description for the new tokens (can be \"\")\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the number of tokens to create\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendissuancefixed", "\"3Ck2kEGLJtZw9ENj2tameMCtS3HB7uRar3\" 2 1 0 \"Companies\" \"CounosH Mining\" \"Quantum Miner\" \"\" \"\" \"1000000\"")
           + HelpExampleRpc("counos_sendissuancefixed", "\"3Ck2kEGLJtZw9ENj2tameMCtS3HB7uRar3\", 2, 1, 0, \"Companies\", \"CounosH Mining\", \"Quantum Miner\", \"\", \"\", \"1000000\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint8_t ecosystem = ParseEcosystem(request.params[1]);
    uint16_t type = ParsePropertyType(request.params[2]);
    uint32_t previousId = ParsePreviousPropertyId(request.params[3]);
    std::string category = ParseText(request.params[4]);
    std::string subcategory = ParseText(request.params[5]);
    std::string name = ParseText(request.params[6]);
    std::string url = ParseText(request.params[7]);
    std::string data = ParseText(request.params[8]);
    int64_t amount = ParseAmount(request.params[9], type);

    // perform checks
    RequirePropertyName(name);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_IssuanceFixed(ecosystem, type, previousId, category, subcategory, name, url, data, amount);
//    std::cout << HexStr(payload) << std::endl;
    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendissuancemanaged(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendissuancemanaged",
       "\nCreate new tokens with manageable supply.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem to create the tokens in (1 for main ecosystem, 2 for test ecosystem)\n"},
           {"type", RPCArg::Type::NUM, RPCArg::Optional::NO, "the type of the tokens to create: (1 for indivisible tokens, 2 for divisible tokens)\n"},
           {"previousid", RPCArg::Type::NUM, RPCArg::Optional::NO, "an identifier of a predecessor token (use 0 for new tokens)\n"},
           {"category", RPCArg::Type::STR, RPCArg::Optional::NO, "a category for the new tokens (can be \"\")\n"},
           {"subcategory", RPCArg::Type::STR, RPCArg::Optional::NO, "a subcategory for the new tokens  (can be \"\")\n"},
           {"name", RPCArg::Type::STR, RPCArg::Optional::NO, "the name of the new tokens to create\n"},
           {"url", RPCArg::Type::STR, RPCArg::Optional::NO, "a URL for further information about the new tokens (can be \"\")\n"},
           {"data", RPCArg::Type::STR, RPCArg::Optional::NO, "a description for the new tokens (can be \"\")\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendissuancemanaged", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\" 2 1 0 \"Companies\" \"CounosH Mining\" \"Quantum Miner\" \"\" \"\"")
           + HelpExampleRpc("counos_sendissuancemanaged", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\", 2, 1, 0, \"Companies\", \"CounosH Mining\", \"Quantum Miner\", \"\", \"\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint8_t ecosystem = ParseEcosystem(request.params[1]);
    uint16_t type = ParsePropertyType(request.params[2]);
    uint32_t previousId = ParsePreviousPropertyId(request.params[3]);
    std::string category = ParseText(request.params[4]);
    std::string subcategory = ParseText(request.params[5]);
    std::string name = ParseText(request.params[6]);
    std::string url = ParseText(request.params[7]);
    std::string data = ParseText(request.params[8]);

    // perform checks
    RequirePropertyName(name);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_IssuanceManaged(ecosystem, type, previousId, category, subcategory, name, url, data);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendsto(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendsto",
       "\nCreate and broadcast a send-to-owners transaction.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to distribute\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount to distribute\n"},
           {"redeemaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "an address that can spend the transaction dust (sender by default)\n"},
           {"distributionproperty", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "the identifier of the property holders to distribute to\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendsto", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\" 3 \"5000\"")
           + HelpExampleRpc("counos_sendsto", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", \"cch1qs2twe7v4tk2uxk59a3nazcp3kdzznj0kt75e07\", 3, \"5000\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);
    int64_t amount = ParseAmount(request.params[2], isPropertyDivisible(propertyId));
    std::string redeemAddress = (request.params.size() > 3 && !ParseText(request.params[3]).empty()) ? ParseAddress(request.params[3]): "";
    uint32_t distributionPropertyId = (request.params.size() > 4) ? ParsePropertyId(request.params[4]) : propertyId;

    // perform checks
    RequireBalance(fromAddress, propertyId, amount);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_SendToOwners(propertyId, amount, distributionPropertyId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", redeemAddress, 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_SEND_TO_OWNERS, propertyId, amount);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendgrant(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendgrant",
       "\nIssue or grant new units of managed tokens.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the receiver of the tokens (sender by default, can be \"\")\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to grant\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to create\n"},
           {"memo", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a text note attached to this transaction (none by default)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendgrant", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\" \"\" 51 \"7000\"")
           + HelpExampleRpc("counos_sendgrant", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\", \"\", 51, \"7000\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = !ParseText(request.params[1]).empty() ? ParseAddress(request.params[1]): "";
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));
    std::string memo = (request.params.size() > 4) ? ParseText(request.params[4]): "";

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_Grant(propertyId, amount, memo);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendrevoke(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendrevoke",
       "\nRevoke units of managed tokens.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to revoke the tokens from\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to revoke\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to revoke\n"},
           {"memo", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "a text note attached to this transaction (none by default)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendrevoke", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\" \"\" 51 \"100\"")
           + HelpExampleRpc("counos_sendrevoke", "\"3HsJvhr9qzgRe3ss97b1QHs38rmaLExLcH\", \"\", 51, \"100\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);
    int64_t amount = ParseAmount(request.params[2], isPropertyDivisible(propertyId));
    std::string memo = (request.params.size() > 3) ? ParseText(request.params[3]): "";

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);
    RequireBalance(fromAddress, propertyId, amount);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_Revoke(propertyId, amount, memo);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendclosecrowdsale(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendclosecrowdsale",
       "\nManually close a crowdsale.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address associated with the crowdsale to close\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the crowdsale to close\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendclosecrowdsale", "\"3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo\" 70")
           + HelpExampleRpc("counos_sendclosecrowdsale", "\"3JYd75REX3HXn1vAU83YuGfmiPXW7BpYXo\", 70")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);

    // perform checks
    RequireExistingProperty(propertyId);
    RequireCrowdsale(propertyId);
    RequireActiveCrowdsale(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_CloseCrowdsale(propertyId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendtrade(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendtrade",
       "\nPlace a trade offer on the distributed token exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to trade with\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to list for sale\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to list for sale\n"},
           {"propertiddesired", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens desired in exchange\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens desired in exchange\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendtrade", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\" 31 \"250.0\" 1 \"10.0\"")
           + HelpExampleRpc("counos_sendtrade", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\", 31, \"250.0\", 1, \"10.0\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = ParseAmount(request.params[2], isPropertyDivisible(propertyIdForSale));
    uint32_t propertyIdDesired = ParsePropertyId(request.params[3]);
    int64_t amountDesired = ParseAmount(request.params[4], isPropertyDivisible(propertyIdDesired));

    // perform checks
    RequireExistingProperty(propertyIdForSale);
    RequireExistingProperty(propertyIdDesired);
    RequireBalance(fromAddress, propertyIdForSale, amountForSale);
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_MetaDExTrade(propertyIdForSale, amountForSale, propertyIdDesired, amountDesired);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_METADEX_TRADE, propertyIdForSale, amountForSale);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendcanceltradesbyprice(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendcanceltradesbyprice",
       "\nCancel offers on the distributed token exchange with the specified price.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to trade with\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens listed for sale\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to listed for sale\n"},
           {"propertiddesired", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens desired in exchange\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens desired in exchange\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendcanceltradesbyprice", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\" 31 \"100.0\" 1 \"5.0\"")
           + HelpExampleRpc("counos_sendcanceltradesbyprice", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\", 31, \"100.0\", 1, \"5.0\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    int64_t amountForSale = ParseAmount(request.params[2], isPropertyDivisible(propertyIdForSale));
    uint32_t propertyIdDesired = ParsePropertyId(request.params[3]);
    int64_t amountDesired = ParseAmount(request.params[4], isPropertyDivisible(propertyIdDesired));

    // perform checks
    RequireExistingProperty(propertyIdForSale);
    RequireExistingProperty(propertyIdDesired);
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);
    // TODO: check, if there are matching offers to cancel

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelPrice(propertyIdForSale, amountForSale, propertyIdDesired, amountDesired);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_METADEX_CANCEL_PRICE, propertyIdForSale, amountForSale, false);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendcanceltradesbypair(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendcanceltradesbypair",
       "\nCancel all offers on the distributed token exchange with the given currency pair.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to trade with\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens listed for sale\n"},
           {"propertiddesired", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens desired in exchange\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendcanceltradesbypair", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\" 1 31")
           + HelpExampleRpc("counos_sendcanceltradesbypair", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\", 1, 31")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[1]);
    uint32_t propertyIdDesired = ParsePropertyId(request.params[2]);

    // perform checks
    RequireExistingProperty(propertyIdForSale);
    RequireExistingProperty(propertyIdDesired);
    RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
    RequireDifferentIds(propertyIdForSale, propertyIdDesired);
    // TODO: check, if there are matching offers to cancel

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelPair(propertyIdForSale, propertyIdDesired);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_METADEX_CANCEL_PAIR, propertyIdForSale, 0, false);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendcancelalltrades(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendcancelalltrades",
       "\nCancel all offers on the distributed token exchange.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to trade with\n"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::NO, "the ecosystem of the offers to cancel (1 for main ecosystem, 2 for test ecosystem)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendcancelalltrades", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\" 1")
           + HelpExampleRpc("counos_sendcancelalltrades", "\"3BydPiSLPP3DR5cf726hDQ89fpqWLxPKLR\", 1")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint8_t ecosystem = ParseEcosystem(request.params[1]);

    // perform checks
    // TODO: check, if there are matching offers to cancel

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_MetaDExCancelEcosystem(ecosystem);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            PendingAdd(txid, fromAddress, MSC_TYPE_METADEX_CANCEL_ECOSYSTEM, ecosystem, 0, false);
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendchangeissuer(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendchangeissuer",
       "\nChange the issuer on record of the given tokens.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address associated with the tokens\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to transfer administrative control to\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendchangeissuer", "\"1ARjWDkZ7kT9fwjPrjcQyvbXDkEySzKHwu\" \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" 3")
           + HelpExampleRpc("counos_sendchangeissuer", "\"1ARjWDkZ7kT9fwjPrjcQyvbXDkEySzKHwu\", \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", 3")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string toAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);

    // perform checks
    RequireExistingProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_ChangeIssuer(propertyId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendenablefreezing(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendenablefreezing",
       "\nEnables address freezing for a centrally managed property.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the issuer of the tokens\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendenablefreezing", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" 3")
           + HelpExampleRpc("counos_sendenablefreezing", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", 3")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_EnableFreezing(propertyId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_senddisablefreezing(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_senddisablefreezing",
       "\nDisables address freezing for a centrally managed property.\n"
       "\nIMPORTANT NOTE:  Disabling freezing for a property will UNFREEZE all frozen addresses for that property!",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the issuer of the tokens\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_senddisablefreezing", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" 3")
           + HelpExampleRpc("counos_senddisablefreezing", "\"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", 3")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DisableFreezing(propertyId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendfreeze(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendfreeze",
       "\nFreeze an address for a centrally managed token.\n"
       "\nNote: Only the issuer may freeze tokens, and only if the token is of the managed type with the freezing option enabled.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from (must be the issuer of the property)\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to freeze tokens for\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property to freeze tokens for (must be managed type and have freezing option enabled)\n"},
           {"amount", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to freeze (note: this is unused - once frozen an address cannot send any transactions for the property)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendfreeze", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" 1 \"0\"")
           + HelpExampleRpc("counos_sendfreeze", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", 1, \"0\"")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string refAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_FreezeTokens(propertyId, amount, refAddress);

    // request the wallet build the transaction (and if needed commit it)
    // Note: no ref address is sent to WalletTxBuilder as the ref address is contained within the payload
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendunfreeze(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendunfreeze",
       "\nUnfreezes an address for a centrally managed token.\n"
       "\nNote: Only the issuer may unfreeze tokens.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from (must be the issuer of the property)\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to unfreeze tokens for\n"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property to unfreeze tokens for (must be managed type and have freezing option enabled)\n"},
           {"amount", RPCArg::Type::NUM, RPCArg::Optional::NO, "the amount of tokens to unfreeze (note: this is unused - once frozen an address cannot send any transactions for the property)\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendunfreeze", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\" 1 0")
           + HelpExampleRpc("counos_sendunfreeze", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", \"cch1qavgpym8haf78k03qy6ekdjy9pc4ecf8zfzjacv\", 1, 0")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    std::string refAddress = ParseAddress(request.params[1]);
    uint32_t propertyId = ParsePropertyId(request.params[2]);
    int64_t amount = ParseAmount(request.params[3], isPropertyDivisible(propertyId));

    // perform checks
    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);
    RequireTokenIssuer(fromAddress, propertyId);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_UnfreezeTokens(propertyId, amount, refAddress);

    // request the wallet build the transaction (and if needed commit it)
    // Note: no ref address is sent to WalletTxBuilder as the ref address is contained within the payload
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendanydata(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendanydata",
       "\nCreate and broadcast a transaction with an arbitrary payload.\nWhen no receiver is specified, the sender is also considered as receiver.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"data", RPCArg::Type::STR, RPCArg::Optional::NO, "the hex-encoded data\n"},
           {"toaddress", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "the optional address of the receiver\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendanydata", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\" \"646578782032303230\"")
           + HelpExampleRpc("counos_sendanydata", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\", \"646578782032303230\"")
       }
    }.Check(request);

    // obtain parameters
    std::string fromAddress = ParseAddress(request.params[0]);
    std::vector<unsigned char> data = ParseHexV(request.params[1], "data");
    std::string toAddress;
    if (request.params.size() > 2) {
        toAddress = ParseAddressOrEmpty(request.params[2]);
    }

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_AnyData(data);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, toAddress, "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendactivation(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendactivation",
       "\nActivate a protocol feature.\n"
       "\nNote: Counos Core ignores activations from unauthorized sources.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"featureid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the feature to activate\n"},
           {"block", RPCArg::Type::NUM, RPCArg::Optional::NO, "the activation block\n"},
           {"minclientversion", RPCArg::Type::NUM, RPCArg::Optional::NO, "the minimum supported client version\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendactivation", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" 1 370000 999")
           + HelpExampleRpc("counos_sendactivation", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", 1, 370000, 999")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint16_t featureId = request.params[1].get_int();
    uint32_t activationBlock = request.params[2].get_int();
    uint32_t minClientVersion = request.params[3].get_int();

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_ActivateFeature(featureId, activationBlock, minClientVersion);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_senddeactivation(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_senddeactivation",
       "\nDeactivate a protocol feature.  For Emergency Use Only.\n"
       "\nNote: Counos Core ignores deactivations from unauthorized sources.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"featureid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the feature to activate\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_senddeactivation", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" 1")
           + HelpExampleRpc("counos_senddeactivation", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", 1")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    uint16_t featureId = request.params[1].get_int64();

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_DeactivateFeature(featureId);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}

static UniValue counos_sendalert(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pwallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_sendalert",
       "\nCreates and broadcasts an Counos Core alert.\n"
       "\nNote: Counos Core ignores alerts from unauthorized sources.\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"alerttype", RPCArg::Type::NUM, RPCArg::Optional::NO, "the alert type\n"},
           {"expiryvalue", RPCArg::Type::NUM, RPCArg::Optional::NO, "the value when the alert expires (depends on alert type)\n"},
           {"message", RPCArg::Type::STR, RPCArg::Optional::NO, "the user-faced alert message\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           HelpExampleCli("counos_sendalert", "")
           + HelpExampleRpc("counos_sendalert", "")
       }
    }.Check(request);

    // obtain parameters & info
    std::string fromAddress = ParseAddress(request.params[0]);
    int64_t tempAlertType = request.params[1].get_int64();
    if (tempAlertType < 1 || 65535 < tempAlertType) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Alert type is out of range");
    }
    uint16_t alertType = static_cast<uint16_t>(tempAlertType);
    int64_t tempExpiryValue = request.params[2].get_int64();
    if (tempExpiryValue < 1 || 4294967295LL < tempExpiryValue) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Expiry value is out of range");
    }
    uint32_t expiryValue = static_cast<uint32_t>(tempExpiryValue);
    std::string alertMessage = ParseText(request.params[3]);

    // create a payload for the transaction
    std::vector<unsigned char> payload = CreatePayload_CounosCoreAlert(alertType, expiryValue, alertMessage);

    // request the wallet build the transaction (and if needed commit it)
    uint256 txid;
    std::string rawHex;
    int result = WalletTxBuilder(fromAddress, "", "", 0, payload, txid, rawHex, autoCommit, pwallet.get());

    // check error and return the txid (or raw hex depending on autocommit)
    if (result != 0) {
        throw JSONRPCError(result, error_str(result));
    } else {
        if (!autoCommit) {
            return rawHex;
        } else {
            return txid.GetHex();
        }
    }
}


static UniValue trade_MP(const JSONRPCRequest& request)
{
    RPCHelpMan{"trade_MP",
       "\nNote: this command is deprecated, and was replaced by:\n",
       {
           {"fromaddress", RPCArg::Type::STR, RPCArg::Optional::NO, "the address to send from\n"},
           {"propertyidforsale", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens to list for sale\n"},
           {"amountforsale", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens to listed for sale\n"},
           {"propertyiddesired", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens desired in exchange\n"},
           {"amountdesired", RPCArg::Type::STR, RPCArg::Optional::NO, "the amount of tokens desired in exchange\n"},
           {"action", RPCArg::Type::NUM, RPCArg::Optional::NO, "trade action to take\n"},
       },
       RPCResult{
           RPCResult::Type::STR_HEX, "hash", "the hex-encoded transaction hash"
       },
       RPCExamples{
           " - sendtrade_COUNOS\n"
           " - sendcanceltradebyprice_COUNOS\n"
           " - sendcanceltradebypair_COUNOS\n"
           " - sendcanceltradebypair_COUNOS\n"
       }
    }.Check(request);

    UniValue values(UniValue::VARR);
    uint8_t action = ParseMetaDExAction(request.params[5]);

    // Forward to the new commands, based on action value
    switch (action) {
        case CMPTransaction::ADD:
        {
            values.push_back(request.params[0]); // fromAddress
            values.push_back(request.params[1]); // propertyIdForSale
            values.push_back(request.params[2]); // amountForSale
            values.push_back(request.params[3]); // propertyIdDesired
            values.push_back(request.params[4]); // amountDesired
            return counos_sendtrade(request);
        }
        case CMPTransaction::CANCEL_AT_PRICE:
        {
            values.push_back(request.params[0]); // fromAddress
            values.push_back(request.params[1]); // propertyIdForSale
            values.push_back(request.params[2]); // amountForSale
            values.push_back(request.params[3]); // propertyIdDesired
            values.push_back(request.params[4]); // amountDesired
            return counos_sendcanceltradesbyprice(request);
        }
        case CMPTransaction::CANCEL_ALL_FOR_PAIR:
        {
            values.push_back(request.params[0]); // fromAddress
            values.push_back(request.params[1]); // propertyIdForSale
            values.push_back(request.params[3]); // propertyIdDesired
            return counos_sendcanceltradesbypair(request);
        }
        case CMPTransaction::CANCEL_EVERYTHING:
        {
            uint8_t ecosystem = 0;
            if (isMainEcosystemProperty(request.params[1].get_int64())
                    && isMainEcosystemProperty(request.params[3].get_int64())) {
                ecosystem = COUNOS_PROPERTY_MSC;
            }
            if (isTestEcosystemProperty(request.params[1].get_int64())
                    && isTestEcosystemProperty(request.params[3].get_int64())) {
                ecosystem = COUNOS_PROPERTY_TMSC;
            }
            values.push_back(request.params[0]); // fromAddress
            values.push_back(ecosystem);
            return counos_sendcancelalltrades(request);
        }
    }

    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid action (1,2,3,4 only)");
}

static const CRPCCommand commands[] =
{ //  category                             name                            actor (function)               okSafeMode
  //  ------------------------------------ ------------------------------- ------------------------------ ----------
    { "counos layer (transaction creation)", "counos_sendrawtx",               &counos_sendrawtx,               {"fromaddress", "rawtransaction", "referenceaddress", "redeemaddress", "referenceamount"} },
    { "counos layer (transaction creation)", "counos_send",                    &counos_send,                    {"fromaddress", "toaddress", "propertyid", "amount", "redeemaddress", "referenceamount"} },
    { "counos layer (transaction creation)", "counos_senddexsell",             &counos_senddexsell,             {"fromaddress", "propertyidforsale", "amountforsale", "amountdesired", "paymentwindow", "minacceptfee", "action"} },
    { "counos layer (transaction creation)", "counos_sendnewdexorder",         &counos_sendnewdexorder,         {"fromaddress", "propertyidforsale", "amountforsale", "amountdesired", "paymentwindow", "minacceptfee"} },
    { "counos layer (transaction creation)", "counos_sendupdatedexorder",      &counos_sendupdatedexorder,      {"fromaddress", "propertyidforsale", "amountforsale", "amountdesired", "paymentwindow", "minacceptfee"} },
    { "counos layer (transaction creation)", "counos_sendcanceldexorder",      &counos_sendcanceldexorder,      {"fromaddress", "propertyidforsale"} },
    { "counos layer (transaction creation)", "counos_senddexaccept",           &counos_senddexaccept,           {"fromaddress", "toaddress", "propertyid", "amount", "override"} },
    { "counos layer (transaction creation)", "counos_senddexpay",              &counos_senddexpay,              {"fromaddress", "toaddress", "propertyid", "amount"} },
    { "counos layer (transaction creation)", "counos_sendissuancecrowdsale",   &counos_sendissuancecrowdsale,   {"fromaddress", "ecosystem", "type", "previousid", "category", "subcategory", "name", "url", "data", "propertyiddesired", "tokensperunit", "deadline", "earlybonus", "issuerpercentage"} },
    { "counos layer (transaction creation)", "counos_sendissuancefixed",       &counos_sendissuancefixed,       {"fromaddress", "ecosystem", "type", "previousid", "category", "subcategory", "name", "url", "data", "amount"} },
    { "counos layer (transaction creation)", "counos_sendissuancemanaged",     &counos_sendissuancemanaged,     {"fromaddress", "ecosystem", "type", "previousid", "category", "subcategory", "name", "url", "data"} },
    { "counos layer (transaction creation)", "counos_sendtrade",               &counos_sendtrade,               {"fromaddress", "propertyidforsale", "amountforsale", "propertiddesired", "amountdesired"} },
    { "counos layer (transaction creation)", "counos_sendcanceltradesbyprice", &counos_sendcanceltradesbyprice, {"fromaddress", "propertyidforsale", "amountforsale", "propertiddesired", "amountdesired"} },
    { "counos layer (transaction creation)", "counos_sendcanceltradesbypair",  &counos_sendcanceltradesbypair,  {"fromaddress", "propertyidforsale", "propertiddesired"} },
    { "counos layer (transaction creation)", "counos_sendcancelalltrades",     &counos_sendcancelalltrades,     {"fromaddress", "ecosystem"} },
    { "counos layer (transaction creation)", "counos_sendsto",                 &counos_sendsto,                 {"fromaddress", "propertyid", "amount", "redeemaddress", "distributionproperty"} },
    { "counos layer (transaction creation)", "counos_sendgrant",               &counos_sendgrant,               {"fromaddress", "toaddress", "propertyid", "amount", "memo"} },
    { "counos layer (transaction creation)", "counos_sendrevoke",              &counos_sendrevoke,              {"fromaddress", "propertyid", "amount", "memo"} },
    { "counos layer (transaction creation)", "counos_sendclosecrowdsale",      &counos_sendclosecrowdsale,      {"fromaddress", "propertyid"} },
    { "counos layer (transaction creation)", "counos_sendchangeissuer",        &counos_sendchangeissuer,        {"fromaddress", "toaddress", "propertyid"} },
    { "counos layer (transaction creation)", "counos_sendall",                 &counos_sendall,                 {"fromaddress", "toaddress", "ecosystem", "redeemaddress", "referenceamount"} },
    { "counos layer (transaction creation)", "counos_sendenablefreezing",      &counos_sendenablefreezing,      {"fromaddress", "propertyid"} },
    { "counos layer (transaction creation)", "counos_senddisablefreezing",     &counos_senddisablefreezing,     {"fromaddress", "propertyid"} },
    { "counos layer (transaction creation)", "counos_sendfreeze",              &counos_sendfreeze,              {"fromaddress", "toaddress", "propertyid", "amount"} },
    { "counos layer (transaction creation)", "counos_sendunfreeze",            &counos_sendunfreeze,            {"fromaddress", "toaddress", "propertyid", "amount"} },
    { "counos layer (transaction creation)", "counos_sendanydata",             &counos_sendanydata,             {"fromaddress", "data", "toaddress"} },
    { "hidden",                            "counos_senddeactivation",        &counos_senddeactivation,        {"fromaddress", "featureid"} },
    { "hidden",                            "counos_sendactivation",          &counos_sendactivation,          {"fromaddress", "featureid", "block", "minclientversion"} },
    { "hidden",                            "counos_sendalert",               &counos_sendalert,               {"fromaddress", "alerttype", "expiryvalue", "message"} },
    { "counos layer (transaction creation)", "counos_funded_send",             &counos_funded_send,             {"fromaddress", "toaddress", "propertyid", "amount", "feeaddress"} },
    { "counos layer (transaction creation)", "counos_funded_sendall",          &counos_funded_sendall,          {"fromaddress", "toaddress", "ecosystem", "feeaddress"} },

    /* deprecated: */
    { "hidden",                            "sendrawtx_MP",                 &counos_sendrawtx,               {"fromaddress", "rawtransaction", "referenceaddress", "redeemaddress", "referenceamount"} },
    { "hidden",                            "send_MP",                      &counos_send,                    {"fromaddress", "toaddress", "propertyid", "amount", "redeemaddress", "referenceamount"} },
    { "hidden",                            "sendtoowners_MP",              &counos_sendsto,                 {"fromaddress", "propertyid", "amount", "redeemaddress", "distributionproperty"} },
    { "hidden",                            "trade_MP",                     &trade_MP,                     {"fromaddress", "propertyidforsale", "amountforsale", "propertiddesired", "amountdesired", "action"} },
};

void RegisterCounosTransactionCreationRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
