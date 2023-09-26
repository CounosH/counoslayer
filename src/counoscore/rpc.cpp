/**
 * @file rpc.cpp
 *
 * This file contains RPC calls for data retrieval.
 */

#include <counoscore/rpc.h>

#include <counoscore/activation.h>
#include <counoscore/consensushash.h>
#include <counoscore/convert.h>
#include <counoscore/dbfees.h>
#include <counoscore/dbspinfo.h>
#include <counoscore/dbstolist.h>
#include <counoscore/dbtradelist.h>
#include <counoscore/dbtxlist.h>
#include <counoscore/dex.h>
#include <counoscore/errors.h>
#include <counoscore/log.h>
#include <counoscore/mdex.h>
#include <counoscore/notifications.h>
#include <counoscore/counoscore.h>
#include <counoscore/parsing.h>
#include <counoscore/rpcrequirements.h>
#include <counoscore/rpctxobject.h>
#include <counoscore/rpcvalues.h>
#include <counoscore/rules.h>
#include <counoscore/sp.h>
#include <counoscore/sto.h>
#include <counoscore/tally.h>
#include <counoscore/tx.h>
#include <counoscore/nftdb.h>
#include <counoscore/utilscounosh.h>
#include <counoscore/version.h>
#include <counoscore/walletfetchtxs.h>
#include <counoscore/walletutils.h>

#include <amount.h>
#include <base58.h>
#include <chainparams.h>
#include <init.h>
#include <index/txindex.h>
#include <interfaces/wallet.h>
#include <key_io.h>
#include <validation.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <rpc/server.h>
#include <rpc/util.h>
#include <tinyformat.h>
#include <txmempool.h>
#include <uint256.h>
#include <util/strencodings.h>
#include <wallet/rpcwallet.h>
#ifdef ENABLE_WALLET
#include <wallet/wallet.h>
#endif

#include <univalue.h>

#include <stdint.h>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>

#include <boost/algorithm/string.hpp> // boost::split

using std::runtime_error;
using namespace mastercore;

/**
 * Throws a JSONRPCError, depending on error code.
 */
void PopulateFailure(int error)
{
    switch (error) {
        case MP_TX_NOT_FOUND:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");
        case MP_TX_UNCONFIRMED:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Unconfirmed transactions are not supported");
        case MP_BLOCK_NOT_IN_CHAIN:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Transaction not part of the active chain");
        case MP_CROWDSALE_WITHOUT_PROPERTY:
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Potential database corruption: \
                                                  \"Crowdsale Purchase\" without valid property identifier");
        case MP_INVALID_TX_IN_DB_FOUND:
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Potential database corruption: Invalid transaction found");
        case MP_TX_IS_NOT_COUNOS_PROTOCOL:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No Counos Layer Protocol transaction");
        case MP_TXINDEX_STILL_SYNCING:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No such mempool transaction. Blockchain transactions are still in the process of being indexed.");
        case MP_RPC_DECODE_INPUTS_MISSING:
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Transaction inputs were not found. Please provide inputs explicitly (see help description) or fully synchronize node.");

    }
    throw JSONRPCError(RPC_INTERNAL_ERROR, "Generic transaction population failure");
}

void PropertyToJSON(const CMPSPInfo::Entry& sProperty, UniValue& property_obj)
{
    property_obj.pushKV("name", sProperty.name);
    property_obj.pushKV("category", sProperty.category);
    property_obj.pushKV("subcategory", sProperty.subcategory);
    property_obj.pushKV("data", sProperty.data);
    property_obj.pushKV("url", sProperty.url);
    property_obj.pushKV("divisible", sProperty.isDivisible());
    property_obj.pushKV("issuer", sProperty.issuer);
    property_obj.pushKV("delegate", sProperty.delegate);
    property_obj.pushKV("creationtxid", sProperty.txid.GetHex());
    property_obj.pushKV("fixedissuance", sProperty.fixed);
    property_obj.pushKV("managedissuance", sProperty.manual);
    property_obj.pushKV("non-fungibletoken", sProperty.unique);
}

void MetaDexObjectToJSON(const CMPMetaDEx& obj, UniValue& metadex_obj)
{
    bool propertyIdForSaleIsDivisible = isPropertyDivisible(obj.getProperty());
    bool propertyIdDesiredIsDivisible = isPropertyDivisible(obj.getDesProperty());
    // add data to JSON object
    metadex_obj.pushKV("address", obj.getAddr());
    metadex_obj.pushKV("txid", obj.getHash().GetHex());
    if (obj.getAction() == 4) metadex_obj.pushKV("ecosystem", isTestEcosystemProperty(obj.getProperty()) ? "test" : "main");
    metadex_obj.pushKV("propertyidforsale", (uint64_t) obj.getProperty());
    metadex_obj.pushKV("propertyidforsaleisdivisible", propertyIdForSaleIsDivisible);
    metadex_obj.pushKV("amountforsale", FormatMP(obj.getProperty(), obj.getAmountForSale()));
    metadex_obj.pushKV("amountremaining", FormatMP(obj.getProperty(), obj.getAmountRemaining()));
    metadex_obj.pushKV("propertyiddesired", (uint64_t) obj.getDesProperty());
    metadex_obj.pushKV("propertyiddesiredisdivisible", propertyIdDesiredIsDivisible);
    metadex_obj.pushKV("amountdesired", FormatMP(obj.getDesProperty(), obj.getAmountDesired()));
    metadex_obj.pushKV("amounttofill", FormatMP(obj.getDesProperty(), obj.getAmountToFill()));
    metadex_obj.pushKV("action", (int) obj.getAction());
    metadex_obj.pushKV("block", obj.getBlock());
    metadex_obj.pushKV("blocktime", obj.getBlockTime());
}

void MetaDexObjectsToJSON(std::vector<CMPMetaDEx>& vMetaDexObjs, UniValue& response)
{
    MetaDEx_compare compareByHeight;

    // sorts metadex objects based on block height and position in block
    std::sort (vMetaDexObjs.begin(), vMetaDexObjs.end(), compareByHeight);

    for (std::vector<CMPMetaDEx>::const_iterator it = vMetaDexObjs.begin(); it != vMetaDexObjs.end(); ++it) {
        UniValue metadex_obj(UniValue::VOBJ);
        MetaDexObjectToJSON(*it, metadex_obj);

        response.push_back(metadex_obj);
    }
}

bool BalanceToJSON(const std::string& address, uint32_t property, UniValue& balance_obj, bool divisible)
{
    // confirmed balance minus unconfirmed, spent amounts
    int64_t nAvailable = GetAvailableTokenBalance(address, property);
    int64_t nReserved = GetReservedTokenBalance(address, property);
    int64_t nFrozen = GetFrozenTokenBalance(address, property);

    if (divisible) {
        balance_obj.pushKV("balance", FormatDivisibleMP(nAvailable));
        balance_obj.pushKV("reserved", FormatDivisibleMP(nReserved));
        balance_obj.pushKV("frozen", FormatDivisibleMP(nFrozen));
    } else {
        balance_obj.pushKV("balance", FormatIndivisibleMP(nAvailable));
        balance_obj.pushKV("reserved", FormatIndivisibleMP(nReserved));
        balance_obj.pushKV("frozen", FormatIndivisibleMP(nFrozen));
    }

    return (nAvailable || nReserved || nFrozen);
}

// display the non-fungible tokens owned by an address for a property
UniValue counos_getnonfungibletokens(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getnonfungibletokens",
               "\nReturns the non-fungible tokens for a given address. Optional property ID filter.\n",
               {
                       {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "the address"},
                       {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "the property identifier"},
               },
               RPCResult{
                       RPCResult::Type::ARR, "", "",
                       {
                               {RPCResult::Type::OBJ, "", "",
                                {
                                        {RPCResult::Type::NUM, "propertyid", "property ID tokens belong to"},
                                        {RPCResult::Type::ARR, "tokens", "",
                                         {
                                                 {RPCResult::Type::OBJ, "", "",
                                                  {
                                                          {RPCResult::Type::NUM, "tokenstart", "the first token in this range"},
                                                          {RPCResult::Type::NUM, "tokenend", "the last token in this range"},
                                                          {RPCResult::Type::NUM, "amount", "the amount of tokens in the range"},
                                                  }}
                                         }}
                                }}
                       }
               },
               RPCExamples{
                       HelpExampleCli("counos_getnonfungibletokens", "\"1EXoDusjGwvnjZUyKkxZ4UHEf77z6A5S4P\" 1")
                       + HelpExampleRpc("counos_getnonfungibletokens", "\"1EXoDusjGwvnjZUyKkxZ4UHEf77z6A5S4P\", 1")
               }
    }.Check(request);

    std::string address = ParseAddress(request.params[0]);
    uint32_t propertyId{0};
    if (!request.params[1].isNull()) {
        propertyId = ParsePropertyId(request.params[1]);
        RequireExistingProperty(propertyId);
        RequireNonFungibleProperty(propertyId);
    }

    UniValue propertyRanges(UniValue::VARR);

    const auto uniqueRanges = pDbNFT->GetAddressNonFungibleTokens(propertyId, address);

    for (const auto& range : uniqueRanges) {
        UniValue property(UniValue::VOBJ);
        property.pushKV("propertyid", static_cast<uint64_t>(range.first));
        UniValue tokenRanges(UniValue::VARR);

        for (const auto& subRange : range.second) {
            UniValue tokenRange(UniValue::VOBJ);
            tokenRange.pushKV("tokenstart", subRange.first);
            tokenRange.pushKV("tokenend", subRange.second);
            int64_t amount = (subRange.second - subRange.first) + 1;
            tokenRange.pushKV("amount", amount);
            tokenRanges.push_back(tokenRange);
        }

        property.pushKV("tokens", tokenRanges);
        propertyRanges.push_back(property);
    }

    return propertyRanges;
}

// provides all data for a specific token
UniValue counos_getnonfungibletokendata(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getnonfungibletokendata",
               "\nReturns owner and all data set in a non-fungible token. If looking\n"
               "up a single token on tokenidstart can be specified only.\n",
               {
                       {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property identifier"},
                       {"tokenidstart", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "the first non-fungible token in range"},
                       {"tokenidend", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "the last non-fungible token in range"},
               },
               RPCResult
                       {RPCResult::Type::ARR, "", "",
                        {
                                {RPCResult::Type::OBJ, "", "",
                                 {
                                         {RPCResult::Type::STR, "owner", "the CounosH address of the owner"},
                                         {RPCResult::Type::STR, "grantdata", "contents of the grant data field"},
                                         {RPCResult::Type::STR, "issuerdata", "contents of the issuer data field"},
                                         {RPCResult::Type::STR, "holderdata", "contents of the holder data field"},
                                 }},
                        }
                       },
               RPCExamples{
                       HelpExampleCli("counos_getnonfungibletokendata", "1 10 20")
                       + HelpExampleRpc("counos_getnonfungibletokendata", "1, 10, 20")
               }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);

    RequireExistingProperty(propertyId);
    RequireNonFungibleProperty(propertyId);

    // Range empty, return null.
    auto range = pDbNFT->GetNonFungibleTokenRanges(propertyId);
    if (range.begin() == range.end()) {
        return NullUniValue;
    }

    // Get token range
    int64_t start{1};
    int64_t end = std::prev(range.end())->second.second;

    if (!request.params[1].isNull()) {
        start = request.params[1].get_int();

        // Make sure start sane.
        if (start < 1) {
            start = 1;
        } else if (start > end) {
            start = end;
        }

        // Allow token start to return single token if end not provided
        if (request.params[2].isNull()) {
            end = start;
        }
    }

    if (!request.params[2].isNull()) {
        end = request.params[2].get_int();

        // Make sure end sane.
        if (end < start) {
            end = start;
        } else if (end > std::prev(range.end())->second.second) {
            end = std::prev(range.end())->second.second;
        }
    }

    UniValue result(UniValue::VARR);
    for (; start <= end; ++start)
    {
        auto owner = pDbNFT->GetNonFungibleTokenOwner(propertyId, start);
        auto grantData = pDbNFT->GetNonFungibleTokenData(propertyId, start, NonFungibleStorage::GrantData);
        auto issuerData = pDbNFT->GetNonFungibleTokenData(propertyId, start, NonFungibleStorage::IssuerData);
        auto holderData = pDbNFT->GetNonFungibleTokenData(propertyId, start, NonFungibleStorage::HolderData);

        UniValue rpcObj(UniValue::VOBJ);
        rpcObj.pushKV("index", start);
        rpcObj.pushKV("owner", owner);
        rpcObj.pushKV("grantdata", grantData);
        rpcObj.pushKV("issuerdata", issuerData);
        rpcObj.pushKV("holderdata", holderData);
        result.push_back(rpcObj);
    }

    return result;
}

// displays all the ranges and their addresses for a property
UniValue counos_getnonfungibletokenranges(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getnonfungibletokenranges",
               "\nReturns the ranges and their addresses for a non-fungible token property.\n",
               {
                       {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property identifier"},
               },
               RPCResult{
                       RPCResult::Type::ARR, "", "",
                       {
                               {RPCResult::Type::OBJ, "", "",
                                {
                                        {RPCResult::Type::STR, "address", "the address"},
                                        {RPCResult::Type::NUM, "tokenstart", "the first token in this range"},
                                        {RPCResult::Type::NUM, "tokenend", "the last token in this range"},
                                        {RPCResult::Type::NUM, "amount", "the amount of tokens in the range"},
                                }},
                       }
               },
               RPCExamples{
                       HelpExampleCli("counos_getnonfungibletokenranges", "1")
                       + HelpExampleRpc("counos_getnonfungibletokenranges", "1")
               }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);

    RequireExistingProperty(propertyId);
    RequireNonFungibleProperty(propertyId);

    UniValue response(UniValue::VARR);

    std::vector<std::pair<std::string,std::pair<int64_t,int64_t> > > rangeMap = pDbNFT->GetNonFungibleTokenRanges(propertyId);

    for (std::vector<std::pair<std::string,std::pair<int64_t,int64_t> > >::iterator it = rangeMap.begin(); it!= rangeMap.end(); ++it) {
        std::pair<std::string,std::pair<int64_t,int64_t> > entry = *it;
        std::string address = entry.first;
        std::pair<int64_t,int64_t> range = entry.second;
        int64_t amount = (range.second - range.first) + 1;

        UniValue uniqueRangeObj(UniValue::VOBJ);
        uniqueRangeObj.pushKV("address", address);
        uniqueRangeObj.pushKV("tokenstart", range.first);
        uniqueRangeObj.pushKV("tokenend", range.second);
        uniqueRangeObj.pushKV("amount", amount);

        response.push_back(uniqueRangeObj);
    }

    return response;
}

// Obtains details of a fee distribution
static UniValue counos_getfeedistribution(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getfeedistribution",
       "\nGet the details for a fee distribution.\n",
       {
           {"distributionid", RPCArg::Type::NUM, RPCArg::Optional::NO, "(number, required) the distribution to obtain details for"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "distributionid", "the distribution id"},
               {RPCResult::Type::NUM, "propertyid", "the property id of the distributed tokens"},
               {RPCResult::Type::NUM, "block", "the block the distribution occurred"},
               {RPCResult::Type::STR_AMOUNT, "amount", "the amount that was distributed"},
               {RPCResult::Type::ARR, "recipients", "a list of recipients",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::STR, "address", "the address of the recipient"},
                       {RPCResult::Type::STR_AMOUNT, "amount", "(string) the amount of fees received by the recipient"},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getfeedistribution", "1")
           + HelpExampleRpc("counos_getfeedistribution", "1")
       }
    }.Check(request);

    int id = request.params[0].get_int();

    int block = 0;
    uint32_t propertyId = 0;
    int64_t total = 0;

    UniValue response(UniValue::VOBJ);

    bool found = pDbFeeHistory->GetDistributionData(id, &propertyId, &block, &total);
    if (!found) {
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Fee distribution ID does not exist");
    }
    response.pushKV("distributionid", id);
    response.pushKV("propertyid", (uint64_t)propertyId);
    response.pushKV("block", block);
    response.pushKV("amount", FormatMP(propertyId, total));
    UniValue recipients(UniValue::VARR);
    std::set<std::pair<std::string,int64_t> > sRecipients = pDbFeeHistory->GetFeeDistribution(id);
    bool divisible = isPropertyDivisible(propertyId);
    if (!sRecipients.empty()) {
        for (std::set<std::pair<std::string,int64_t> >::iterator it = sRecipients.begin(); it != sRecipients.end(); it++) {
            std::string address = (*it).first;
            int64_t amount = (*it).second;
            UniValue recipient(UniValue::VOBJ);
            recipient.pushKV("address", address);
            if (divisible) {
                recipient.pushKV("amount", FormatDivisibleMP(amount));
            } else {
                recipient.pushKV("amount", FormatIndivisibleMP(amount));
            }
            recipients.push_back(recipient);
        }
    }
    response.pushKV("recipients", recipients);
    return response;
}

// Obtains all fee distributions for a property
// TODO : Split off code to populate a fee distribution object into a separate function
static UniValue counos_getfeedistributions(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getfeedistributions",
       "\nGet the details of all fee distributions for a property.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property id to retrieve distributions for"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
                {RPCResult::Type::OBJ, "", "",
                {
                     {RPCResult::Type::NUM, "distributionid", "the distribution id"},
                     {RPCResult::Type::NUM, "propertyid", "the property id of the distributed tokens"},
                     {RPCResult::Type::NUM, "block", "the block the distribution occurred"},
                     {RPCResult::Type::STR_AMOUNT, "amount", "the amount that was distributed"},
                     {RPCResult::Type::ARR, "recipients", "a list of recipients",
                     {
                         {RPCResult::Type::OBJ, "", "",
                         {
                             {RPCResult::Type::STR, "address", "the address of the recipient"},
                             {RPCResult::Type::STR_AMOUNT, "amount", "(string) the amount of fees received by the recipient"},
                         }},
                     }},
                }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getfeedistributions", "1")
           + HelpExampleRpc("counos_getfeedistributions", "1")
       }
    }.Check(request);

    uint32_t prop = ParsePropertyId(request.params[0]);
    RequireExistingProperty(prop);

    UniValue response(UniValue::VARR);

    std::set<int> sDistributions = pDbFeeHistory->GetDistributionsForProperty(prop);
    if (!sDistributions.empty()) {
        for (std::set<int>::iterator it = sDistributions.begin(); it != sDistributions.end(); it++) {
            int id = *it;
            int block = 0;
            uint32_t propertyId = 0;
            int64_t total = 0;
            UniValue responseObj(UniValue::VOBJ);
            bool found = pDbFeeHistory->GetDistributionData(id, &propertyId, &block, &total);
            if (!found) {
                PrintToLog("Fee History Error - Distribution data not found for distribution ID %d but it was included in GetDistributionsForProperty(prop %d)\n", id, prop);
                continue;
            }
            responseObj.pushKV("distributionid", id);
            responseObj.pushKV("propertyid", (uint64_t)propertyId);
            responseObj.pushKV("block", block);
            responseObj.pushKV("amount", FormatMP(propertyId, total));
            UniValue recipients(UniValue::VARR);
            std::set<std::pair<std::string,int64_t> > sRecipients = pDbFeeHistory->GetFeeDistribution(id);
            bool divisible = isPropertyDivisible(propertyId);
            if (!sRecipients.empty()) {
                for (std::set<std::pair<std::string,int64_t> >::iterator it = sRecipients.begin(); it != sRecipients.end(); it++) {
                    std::string address = (*it).first;
                    int64_t amount = (*it).second;
                    UniValue recipient(UniValue::VOBJ);
                    recipient.pushKV("address", address);
                    if (divisible) {
                        recipient.pushKV("amount", FormatDivisibleMP(amount));
                    } else {
                        recipient.pushKV("amount", FormatIndivisibleMP(amount));
                    }
                    recipients.push_back(recipient);
                }
            }
            responseObj.pushKV("recipients", recipients);
            response.push_back(responseObj);
        }
    }
    return response;
}

// Obtains the trigger value for fee distribution for a/all properties
static UniValue counos_getfeetrigger(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getfeetrigger",
       "\nReturns the amount of fees required in the cache to trigger distribution.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "filter the results on this property id"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                    {RPCResult::Type::NUM, "propertyid", "the property id"},
                    {RPCResult::Type::STR_AMOUNT, "feetrigger", "the amount of fees required to trigger distribution"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getfeetrigger", "3")
           + HelpExampleRpc("counos_getfeetrigger", "3")
       }
    }.Check(request);

    uint32_t propertyId = 0;
    if (0 < request.params.size()) {
        propertyId = ParsePropertyId(request.params[0]);
    }

    if (propertyId > 0) {
        RequireExistingProperty(propertyId);
    }

    UniValue response(UniValue::VARR);

    for (uint8_t ecosystem = 1; ecosystem <= 2; ecosystem++) {
        uint32_t startPropertyId = (ecosystem == 1) ? 1 : TEST_ECO_PROPERTY_1;
        for (uint32_t itPropertyId = startPropertyId; itPropertyId < pDbSpInfo->peekNextSPID(ecosystem); itPropertyId++) {
            if (propertyId == 0 || propertyId == itPropertyId) {
                int64_t feeTrigger = pDbFeeCache->GetDistributionThreshold(itPropertyId);
                std::string strFeeTrigger = FormatMP(itPropertyId, feeTrigger);
                UniValue cacheObj(UniValue::VOBJ);
                cacheObj.pushKV("propertyid", (uint64_t)itPropertyId);
                cacheObj.pushKV("feetrigger", strFeeTrigger);
                response.push_back(cacheObj);
            }
        }
    }

    return response;
}

// Provides the fee share the wallet (or specific address) will receive from fee distributions
#ifdef ENABLE_WALLET
static UniValue counos_getfeeshare(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_getfeeshare",
       "\nReturns the percentage share of fees distribution applied to the wallet (default) or address (if supplied).\n",
       {
           {"address", RPCArg::Type::STR, RPCArg::Optional::OMITTED, "retrieve the fee share for the supplied address"},
           {"ecosystem", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "the ecosystem to check the fee share (1 for main ecosystem, 2 for test ecosystem)"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "address", "the property id"},
                   {RPCResult::Type::STR_AMOUNT, "feeshare", "the percentage of fees this address will receive based on current state"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getfeeshare", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" 1")
           + HelpExampleRpc("counos_getfeeshare", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", 1")
       }
    }.Check(request);

    std::string address;
    uint8_t ecosystem = 1;
    if (0 < request.params.size()) {
        if ("*" != request.params[0].get_str()) { //ParseAddressOrEmpty doesn't take wildcards
            address = ParseAddressOrEmpty(request.params[0]);
        } else {
            address = "*";
        }
    }
    if (1 < request.params.size()) {
        ecosystem = ParseEcosystem(request.params[1]);
    }

    UniValue response(UniValue::VARR);
    bool addObj = false;

    OwnerAddrType receiversSet;
    if (ecosystem == 1) {
        receiversSet = STO_GetReceivers("FEEDISTRIBUTION", COUNOS_PROPERTY_MSC, COIN);
    } else {
        receiversSet = STO_GetReceivers("FEEDISTRIBUTION", COUNOS_PROPERTY_TMSC, COIN);
    }

    for (OwnerAddrType::reverse_iterator it = receiversSet.rbegin(); it != receiversSet.rend(); ++it) {
        addObj = false;
        if (address.empty()) {
            if (IsMyAddress(it->second, pWallet.get())) {
                addObj = true;
            }
        } else if (address == it->second || address == "*") {
            addObj = true;
        }
        if (addObj) {
            UniValue feeShareObj(UniValue::VOBJ);
            // NOTE: using float here as this is a display value only which isn't an exact percentage and
            //       changes block to block (due to dev Counos) so high precision not required(?)
            double feeShare = (double(it->first) / double(COIN)) * (double)100;
            std::string strFeeShare = strprintf("%.4f", feeShare);
            strFeeShare += "%";
            feeShareObj.pushKV("address", it->second);
            feeShareObj.pushKV("feeshare", strFeeShare);
            response.push_back(feeShareObj);
        }
    }

    return response;
}
#endif

// Provides the current values of the fee cache
static UniValue counos_getfeecache(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getfeecache",
       "\nReturns the amount of fees cached for distribution.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "filter the results on this property id"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "propertyid", "the property id"},
                   {RPCResult::Type::STR_AMOUNT, "cachedfees", "the amount of fees cached for this property"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getfeecache", "31")
           + HelpExampleRpc("counos_getfeecache", "31")
       }
    }.Check(request);

    uint32_t propertyId = 0;
    if (0 < request.params.size()) {
        propertyId = ParsePropertyId(request.params[0]);
    }

    if (propertyId > 0) {
        RequireExistingProperty(propertyId);
    }

    UniValue response(UniValue::VARR);

    for (uint8_t ecosystem = 1; ecosystem <= 2; ecosystem++) {
        uint32_t startPropertyId = (ecosystem == 1) ? 1 : TEST_ECO_PROPERTY_1;
        for (uint32_t itPropertyId = startPropertyId; itPropertyId < pDbSpInfo->peekNextSPID(ecosystem); itPropertyId++) {
            if (propertyId == 0 || propertyId == itPropertyId) {
                int64_t cachedFee = pDbFeeCache->GetCachedAmount(itPropertyId);
                if (cachedFee == 0) {
                    // filter empty results unless the call specifically requested this property
                    if (propertyId != itPropertyId) continue;
                }
                std::string strFee = FormatMP(itPropertyId, cachedFee);
                UniValue cacheObj(UniValue::VOBJ);
                cacheObj.pushKV("propertyid", (uint64_t)itPropertyId);
                cacheObj.pushKV("cachedfees", strFee);
                response.push_back(cacheObj);
            }
        }
    }

    return response;
}

// generate a list of seed blocks based on the data in LevelDB
static UniValue counos_getseedblocks(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getseedblocks",
       "\nReturns a list of blocks containing Counos transactions for use in seed block filtering.\n",
       {
           {"startblock", RPCArg::Type::NUM, RPCArg::Optional::NO, "the first block to look for Counos transactions (inclusive)"},
           {"endblock", RPCArg::Type::NUM, RPCArg::Optional::NO, "the last block to look for Counos transactions (inclusive)"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "a list of seed blocks",
           {
               {RPCResult::Type::NUM, "", "the block height of the seed block"},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getseedblocks", "290000 300000")
           + HelpExampleRpc("counos_getseedblocks", "290000, 300000")
       }
    }.Check(request);

    int startHeight = request.params[0].get_int();
    int endHeight = request.params[1].get_int();

    RequireHeightInChain(startHeight);
    RequireHeightInChain(endHeight);

    UniValue response(UniValue::VARR);

    {
        LOCK(cs_tally);
        std::set<int> setSeedBlocks = pDbTransactionList->GetSeedBlocks(startHeight, endHeight);
        for (std::set<int>::const_iterator it = setSeedBlocks.begin(); it != setSeedBlocks.end(); ++it) {
            response.push_back(*it);
        }
    }

    return response;
}

// obtain the payload for a transaction
static UniValue counos_getpayload(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getpayload",
       "\nGet the payload for an Counos transaction.\n",
       {
           {"txid", RPCArg::Type::STR, RPCArg::Optional::NO, "the hash of the transaction to retrieve payload"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::STR, "payload", "the decoded Counos payload message"},
               {RPCResult::Type::NUM, "payloadsize", "the size of the payload"},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getpayload", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
           + HelpExampleRpc("counos_getpayload", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
       }
    }.Check(request);

    uint256 txid = ParseHashV(request.params[0], "txid");

    bool f_txindex_ready = false;
    if (g_txindex) {
        f_txindex_ready = g_txindex->BlockUntilSyncedToCurrentChain();
    }

    CTransactionRef tx;
    uint256 blockHash;
    if (!GetTransaction(txid, tx, Params().GetConsensus(), blockHash)) {
        if (!f_txindex_ready) {
            PopulateFailure(MP_TXINDEX_STILL_SYNCING);
        } else {
            PopulateFailure(MP_TX_NOT_FOUND);
        }
    }

    int blockTime = 0;
    int blockHeight = GetHeight();
    if (!blockHash.IsNull()) {
        CBlockIndex* pBlockIndex = GetBlockIndex(blockHash);
        if (nullptr != pBlockIndex) {
            blockTime = pBlockIndex->nTime;
            blockHeight = pBlockIndex->nHeight;
        }
    }

    CMPTransaction mp_obj;
    int parseRC = ParseTransaction(*tx, blockHeight, 0, mp_obj, blockTime);
    if (parseRC < 0) PopulateFailure(MP_TX_IS_NOT_COUNOS_PROTOCOL);

    UniValue payloadObj(UniValue::VOBJ);
    payloadObj.pushKV("payload", mp_obj.getPayload());
    payloadObj.pushKV("payloadsize", mp_obj.getPayloadSize());
    return payloadObj;
}

// determine whether to automatically commit transactions
#ifdef ENABLE_WALLET
static UniValue counos_setautocommit(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_setautocommit",
       "\nSets the global flag that determines whether transactions are automatically committed and broadcast.\n",
       {
           {"flag", RPCArg::Type::BOOL, RPCArg::Optional::NO, "the flag"},
       },
       RPCResult{
           RPCResult::Type::BOOL, "", "the updated flag status"
       },
       RPCExamples{
           HelpExampleCli("counos_setautocommit", "false")
           + HelpExampleRpc("counos_setautocommit", "false")
       }
    }.Check(request);

    LOCK(cs_tally);

    autoCommit = request.params[0].get_bool();
    return autoCommit;
}
#endif

// display the tally map & the offer/accept list(s)
static UniValue mscrpc(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    CWallet* const pwallet = wallet.get();
#endif

    int extra = 0;
    int extra2 = 0, extra3 = 0;
    if (request.fHelp || request.params.size() > 3)
        throw runtime_error(
            RPCHelpMan{"mscrpc",
               "\nReturns the number of blocks in the longest block chain.\n",
               {},
               RPCResult{
                   RPCResult::Type::NUM, "", "the current block count"
               },
               RPCExamples{
                   HelpExampleCli("mscrpc", "")
                   + HelpExampleRpc("mscrpc", "")
               }
    }.ToString());

    if (0 < request.params.size()) extra = atoi(request.params[0].get_str());
    if (1 < request.params.size()) extra2 = atoi(request.params[1].get_str());
    if (2 < request.params.size()) extra3 = atoi(request.params[2].get_str());

    PrintToConsole("%s(extra=%d,extra2=%d,extra3=%d)\n", __FUNCTION__, extra, extra2, extra3);

    bool bDivisible = isPropertyDivisible(extra2);

    // various extra tests
    switch (extra) {
        case 0:
        {
            LOCK(cs_tally);
            int64_t total = 0;
            // display all balances
            for (std::unordered_map<std::string, CMPTally>::iterator my_it = mp_tally_map.begin(); my_it != mp_tally_map.end(); ++my_it) {
                PrintToConsole("%34s => ", my_it->first);
                total += (my_it->second).print(extra2, bDivisible);
            }
            PrintToConsole("total for property %d  = %X is %s\n", extra2, extra2, FormatDivisibleMP(total));
            break;
        }
        case 1:
        {
            LOCK(cs_tally);
            // display the whole CMPTxList (leveldb)
            pDbTransactionList->printAll();
            pDbTransactionList->printStats();
            break;
        }
        case 2:
        {
            LOCK(cs_tally);
            // display smart properties
            pDbSpInfo->printAll();
            break;
        }
        case 3:
        {
            LOCK(cs_tally);
            uint32_t id = 0;
            // for each address display all currencies it holds
            for (std::unordered_map<std::string, CMPTally>::iterator my_it = mp_tally_map.begin(); my_it != mp_tally_map.end(); ++my_it) {
                PrintToConsole("%34s => ", my_it->first);
                (my_it->second).print(extra2);
                (my_it->second).init();
                while (0 != (id = (my_it->second).next())) {
                    PrintToConsole("Id: %u=0x%X ", id, id);
                }
                PrintToConsole("\n");
            }
            break;
        }
        case 4:
        {
            LOCK(cs_tally);
            for (CrowdMap::const_iterator it = my_crowds.begin(); it != my_crowds.end(); ++it) {
                (it->second).print(it->first);
            }
            break;
        }
        case 5:
        {
            LOCK(cs_tally);
            PrintToConsole("isMPinBlockRange(%d,%d)=%s\n", extra2, extra3, pDbTransactionList->isMPinBlockRange(extra2, extra3, false) ? "YES" : "NO");
            break;
        }
        case 6:
        {
            LOCK(cs_tally);
            MetaDEx_debug_print(true, true);
            break;
        }
        case 7:
        {
            LOCK(cs_tally);
            // display the whole CMPTradeList (leveldb)
            pDbTradeList->printAll();
            pDbTradeList->printStats();
            break;
        }
        case 8:
        {
            LOCK(cs_tally);
            // display the STO receive list
            pDbStoList->printAll();
            pDbStoList->printStats();
            break;
        }
        case 9:
        {
            PrintToConsole("Locking cs_tally for %d milliseconds..\n", extra2);
            LOCK(cs_tally);
            UninterruptibleSleep(std::chrono::milliseconds{extra2});
            PrintToConsole("Unlocking cs_tally now\n");
            break;
        }
        case 10:
        {
            PrintToConsole("Locking cs_main for %d milliseconds..\n", extra2);
            LOCK(cs_main);
            UninterruptibleSleep(std::chrono::milliseconds{extra2});
            PrintToConsole("Unlocking cs_main now\n");
            break;
        }
#ifdef ENABLE_WALLET
        case 11:
        {
            PrintToConsole("Locking pwallet->cs_wallet for %d milliseconds..\n", extra2);
            LOCK(pwallet->cs_wallet);
            UninterruptibleSleep(std::chrono::milliseconds{extra2});
            PrintToConsole("Unlocking pwallet->cs_wallet now\n");
            break;
        }
#endif
        case 13:
        {
            // dump the non-fungible tokens database
            pDbNFT->printAll();
            pDbNFT->printStats();
            break;
        }
        case 14:
        {
            LOCK(cs_tally);
            pDbFeeCache->printAll();
            pDbFeeCache->printStats();

            int64_t a = 1000;
            int64_t b = 1999;
            int64_t c = 2001;
            int64_t d = 20001;
            int64_t e = 19999;

            int64_t z = 2000;

            int64_t aa = a/z;
            int64_t bb = b/z;
            int64_t cc = c/z;
            int64_t dd = d/z;
            int64_t ee = e/z;

            PrintToConsole("%d / %d = %d\n",a,z,aa);
            PrintToConsole("%d / %d = %d\n",b,z,bb);
            PrintToConsole("%d / %d = %d\n",c,z,cc);
            PrintToConsole("%d / %d = %d\n",d,z,dd);
            PrintToConsole("%d / %d = %d\n",e,z,ee);

            break;
        }
        default:
            break;
    }

    return GetHeight();
}

// display an MP balance via RPC
static UniValue counos_getbalance(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getbalance",
       "\nReturns the token balance for a given address and property.\n",
       {
           {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "the address"},
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, ""},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::STR_AMOUNT, "balance", "the available balance of the address"},
               {RPCResult::Type::STR_AMOUNT, "reserved", "the amount reserved by sell offers and accepts"},
               {RPCResult::Type::STR_AMOUNT, "frozen", "the amount frozen by the issuer (applies to managed properties only)"},
           },
       },
       RPCExamples{
           HelpExampleCli("counos_getbalance", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\" 1")
           + HelpExampleRpc("counos_getbalance", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\", 1")
       }
    }.Check(request);

    std::string address = ParseAddress(request.params[0]);
    uint32_t propertyId = ParsePropertyId(request.params[1]);

    RequireExistingProperty(propertyId);

    UniValue balanceObj(UniValue::VOBJ);
    BalanceToJSON(address, propertyId, balanceObj, isPropertyDivisible(propertyId));

    return balanceObj;
}

static UniValue counos_getallbalancesforid(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getallbalancesforid",
       "\nReturns a list of token balances for a given currency or property identifier.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property identifier"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR, "address", "the address"},
                   {RPCResult::Type::STR_AMOUNT, "balance", "the available balance of the address"},
                   {RPCResult::Type::STR_AMOUNT, "reserved", "the amount reserved by sell offers and accepts"},
                   {RPCResult::Type::STR_AMOUNT, "frozen", "the amount frozen by the issuer (applies to managed properties only)"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getallbalancesforid", "1")
           + HelpExampleRpc("counos_getallbalancesforid", "1")
       }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);

    RequireExistingProperty(propertyId);

    UniValue response(UniValue::VARR);
    bool isDivisible = isPropertyDivisible(propertyId); // we want to check this BEFORE the loop

    LOCK(cs_tally);

    for (std::unordered_map<std::string, CMPTally>::iterator it = mp_tally_map.begin(); it != mp_tally_map.end(); ++it) {
        uint32_t id = 0;
        bool includeAddress = false;
        std::string address = it->first;
        (it->second).init();
        while (0 != (id = (it->second).next())) {
            if (id == propertyId) {
                includeAddress = true;
                break;
            }
        }
        if (!includeAddress) {
            continue; // ignore this address, has never transacted in this propertyId
        }
        UniValue balanceObj(UniValue::VOBJ);
        balanceObj.pushKV("address", address);
        bool nonEmptyBalance = BalanceToJSON(address, propertyId, balanceObj, isDivisible);

        if (nonEmptyBalance) {
            response.push_back(balanceObj);
        }
    }

    return response;
}

static UniValue counos_getallbalancesforaddress(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getallbalancesforaddress",
       "\nReturns a list of all token balances for a given address.\n",
       {
           {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "the address"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "propertyid", "the property identifier"},
                   {RPCResult::Type::STR, "name", "the name of the property"},
                   {RPCResult::Type::STR_AMOUNT, "balance", "the available balance of the address"},
                   {RPCResult::Type::STR_AMOUNT, "reserved", "the amount reserved by sell offers and accepts"},
                   {RPCResult::Type::STR_AMOUNT, "frozen", "the amount frozen by the issuer (applies to managed properties only)"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getallbalancesforaddress", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\"")
           + HelpExampleRpc("counos_getallbalancesforaddress", "\"cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam\"")
       }
    }.Check(request);

    std::string address = ParseAddress(request.params[0]);

    UniValue response(UniValue::VARR);

    LOCK(cs_tally);

    CMPTally* addressTally = getTally(address);

    if (nullptr == addressTally) { // addressTally object does not exist
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Address not found");
    }

    addressTally->init();

    uint32_t propertyId = 0;
    while (0 != (propertyId = addressTally->next())) {
        CMPSPInfo::Entry property;
        if (!pDbSpInfo->getSP(propertyId, property)) {
            continue;
        }

        UniValue balanceObj(UniValue::VOBJ);
        balanceObj.pushKV("propertyid", (uint64_t) propertyId);
        balanceObj.pushKV("name", property.name);

        bool nonEmptyBalance = BalanceToJSON(address, propertyId, balanceObj, property.isDivisible());

        if (nonEmptyBalance) {
            response.push_back(balanceObj);
        }
    }

    return response;
}

/** Returns all addresses that may be mine. */
static std::set<std::string> getWalletAddresses(const JSONRPCRequest& request, bool fIncludeWatchOnly)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    CWallet* const pwallet = wallet.get();
#endif

    std::set<std::string> result;

#ifdef ENABLE_WALLET
    LOCK(pwallet->cs_wallet);

    for(const auto& item : pwallet->m_address_book) {
        const CTxDestination& address = item.first;
        isminetype iIsMine = pwallet->IsMine(address);

        if (iIsMine == ISMINE_SPENDABLE || (fIncludeWatchOnly && iIsMine != ISMINE_NO)) {
            result.insert(EncodeDestination(address));
        }
    }
#endif

    return result;
}

#ifdef ENABLE_WALLET
static UniValue counos_getwalletbalances(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    CWallet* const pwallet = wallet.get();

    RPCHelpMan{"counos_getwalletbalances",
       "\nReturns a list of the total token balances of the whole wallet.\n",
       {
           {"includewatchonly", RPCArg::Type::BOOL, /* default */ "false", "include balances of watchonly addresses"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "propertyid", "the property identifier"},
                   {RPCResult::Type::STR, "name", "the name of the token"},
                   {RPCResult::Type::STR_AMOUNT, "balance", "the total available balance of the token"},
                   {RPCResult::Type::STR_AMOUNT, "reserved", "the total amount reserved by sell offers and accepts"},
                   {RPCResult::Type::STR_AMOUNT, "frozen", "the total amount frozen by the issuer (applies to managed properties only)"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getwalletbalances", "")
           + HelpExampleRpc("counos_getwalletbalances", "")
       }
    }.Check(request);

    bool fIncludeWatchOnly = false;
    if (request.params.size() > 0) {
        fIncludeWatchOnly = request.params[0].get_bool();
    }

    UniValue response(UniValue::VARR);

    if (!pwallet) {
        return response;
    }

    std::set<std::string> addresses = getWalletAddresses(request, fIncludeWatchOnly);
    std::map<uint32_t, std::tuple<int64_t, int64_t, int64_t>> balances;

    LOCK(cs_tally);
    for(const std::string& address : addresses) {
        CMPTally* addressTally = getTally(address);
        if (nullptr == addressTally) {
            continue; // address doesn't have tokens
        }

        uint32_t propertyId = 0;
        addressTally->init();

        while (0 != (propertyId = addressTally->next())) {
            int64_t nAvailable = GetAvailableTokenBalance(address, propertyId);
            int64_t nReserved = GetReservedTokenBalance(address, propertyId);
            int64_t nFrozen = GetFrozenTokenBalance(address, propertyId);

            if (!nAvailable && !nReserved && !nFrozen) {
                continue;
            }

            std::tuple<int64_t, int64_t, int64_t> currentBalance{0, 0, 0};
            if (balances.find(propertyId) != balances.end()) {
                currentBalance = balances[propertyId];
            }

            currentBalance = std::make_tuple(
                    std::get<0>(currentBalance) + nAvailable,
                    std::get<1>(currentBalance) + nReserved,
                    std::get<2>(currentBalance) + nFrozen
            );

            balances[propertyId] = currentBalance;
        }
    }

    for (auto const& item : balances) {
        uint32_t propertyId = item.first;
        std::tuple<int64_t, int64_t, int64_t> balance = item.second;

        CMPSPInfo::Entry property;
        if (!pDbSpInfo->getSP(propertyId, property)) {
            continue; // token wasn't found in the DB
        }

        int64_t nAvailable = std::get<0>(balance);
        int64_t nReserved = std::get<1>(balance);
        int64_t nFrozen = std::get<2>(balance);

        UniValue objBalance(UniValue::VOBJ);
        objBalance.pushKV("propertyid", (uint64_t) propertyId);
        objBalance.pushKV("name", property.name);

        if (property.isDivisible()) {
            objBalance.pushKV("balance", FormatDivisibleMP(nAvailable));
            objBalance.pushKV("reserved", FormatDivisibleMP(nReserved));
            objBalance.pushKV("frozen", FormatDivisibleMP(nFrozen));
        } else {
            objBalance.pushKV("balance", FormatIndivisibleMP(nAvailable));
            objBalance.pushKV("reserved", FormatIndivisibleMP(nReserved));
            objBalance.pushKV("frozen", FormatIndivisibleMP(nFrozen));
        }

        response.push_back(objBalance);
    }

    return response;
}

static UniValue counos_getwalletaddressbalances(const JSONRPCRequest& request)
{

    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    CWallet* const pwallet = wallet.get();

    RPCHelpMan{"counos_getwalletaddressbalances",
       "\nReturns a list of all token balances for every wallet address.\n",
       {
           {"includewatchonly", RPCArg::Type::BOOL, /*default */ "false", "include balances of watchonly addresses"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR, "address", "the address linked to the following balances"},
                   {RPCResult::Type::ARR, "balances", "",
                   {
                       {RPCResult::Type::OBJ, "", "",
                       {
                           {RPCResult::Type::NUM, "propertyid", "the property identifier"},
                           {RPCResult::Type::STR, "name", "the name of the token"},
                           {RPCResult::Type::STR_AMOUNT, "balance", "the available balance of the token"},
                           {RPCResult::Type::STR_AMOUNT, "reserved", "the amount reserved by sell offers and accepts"},
                           {RPCResult::Type::STR_AMOUNT, "frozen", "the amount frozen by the issuer (applies to managed properties only)"},
                       }},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getwalletaddressbalances", "")
           + HelpExampleRpc("counos_getwalletaddressbalances", "")
       }
    }.Check(request);

    bool fIncludeWatchOnly = false;
    if (request.params.size() > 0) {
        fIncludeWatchOnly = request.params[0].get_bool();
    }

    UniValue response(UniValue::VARR);

    if (!pwallet) {
        return response;
    }

    std::set<std::string> addresses = getWalletAddresses(request, fIncludeWatchOnly);

    LOCK(cs_tally);
    for(const std::string& address : addresses) {
        CMPTally* addressTally = getTally(address);
        if (nullptr == addressTally) {
            continue; // address doesn't have tokens
        }

        UniValue arrBalances(UniValue::VARR);
        uint32_t propertyId = 0;
        addressTally->init();

        while (0 != (propertyId = addressTally->next())) {
            CMPSPInfo::Entry property;
            if (!pDbSpInfo->getSP(propertyId, property)) {
                continue; // token wasn't found in the DB
            }

            UniValue objBalance(UniValue::VOBJ);
            objBalance.pushKV("propertyid", (uint64_t) propertyId);
            objBalance.pushKV("name", property.name);

            bool nonEmptyBalance = BalanceToJSON(address, propertyId, objBalance, property.isDivisible());

            if (nonEmptyBalance) {
                arrBalances.push_back(objBalance);
            }
        }

        if (arrBalances.size() > 0) {
            UniValue objEntry(UniValue::VOBJ);
            objEntry.pushKV("address", address);
            objEntry.pushKV("balances", arrBalances);
            response.push_back(objEntry);
        }
    }

    return response;
}
#endif

static UniValue counos_getproperty(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getproperty",
       "\nReturns details for about the tokens or smart property to lookup.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the tokens or property"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "propertyid", "the property identifier"},
               {RPCResult::Type::STR, "name", "the name of the token"},
               {RPCResult::Type::STR, "category", "the category used for the tokens"},
               {RPCResult::Type::STR, "subcategory", "the subcategory used for the tokens"},
               {RPCResult::Type::STR, "data", "additional information or a description"},
               {RPCResult::Type::STR, "url", "a URI, for example pointing to a website"},
               {RPCResult::Type::BOOL, "divisible", "whether the tokens are divisible"},
               {RPCResult::Type::STR, "issuer", "the CounosH address of the issuer on record"},
               {RPCResult::Type::STR, "delegate", "the CounosH address of the issuance delegate, if there is one"},
               {RPCResult::Type::STR_HEX, "creationtxid", "the hex-encoded creation transaction hash"},
               {RPCResult::Type::BOOL, "fixedissuance", "whether the token supply is fixed"},
               {RPCResult::Type::BOOL, "managedissuance", "whether the token supply is managed"},
               {RPCResult::Type::BOOL, "non-fungibletoken", "whether the property contains non-fungible tokens"},
               {RPCResult::Type::BOOL, "freezingenabled", "whether freezing is enabled for the property (managed properties only)"},
               {RPCResult::Type::STR_AMOUNT, "totaltokens", "the total number of tokens in existence"},
           },
       },
       RPCExamples{
           HelpExampleCli("counos_getproperty", "3")
           + HelpExampleRpc("counos_getproperty", "3")
       }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);

    RequireExistingProperty(propertyId);

    CMPSPInfo::Entry sp;
    {
        LOCK(cs_tally);
        if (!pDbSpInfo->getSP(propertyId, sp)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Property identifier does not exist");
        }
    }
    int64_t nTotalTokens = getTotalTokens(propertyId);
    std::string strTotalTokens = FormatMP(propertyId, nTotalTokens);

    UniValue response(UniValue::VOBJ);
    response.pushKV("propertyid", (uint64_t) propertyId);
    PropertyToJSON(sp, response); // name, category, subcategory, ...

    if (sp.manual) {
        int currentBlock = GetHeight();
        LOCK(cs_tally);
        response.pushKV("freezingenabled", isFreezingEnabled(propertyId, currentBlock));
    }
    response.pushKV("totaltokens", strTotalTokens);

    return response;
}

static UniValue counos_listproperties(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_listproperties",
       "\nLists all tokens or smart properties. To get the total number of tokens, please use counos_getproperty.\n",
       {},
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                    {RPCResult::Type::NUM, "propertyid", "the identifier of the tokens"},
                    {RPCResult::Type::STR, "name", "the name of the tokens"},
                    {RPCResult::Type::STR, "category", "the category used for the tokens"},
                    {RPCResult::Type::STR, "subcategory", "the subcategory used for the tokens"},
                    {RPCResult::Type::STR, "data", "additional information or a description"},
                    {RPCResult::Type::STR, "url", "a URI, for example pointing to a website"},
                    {RPCResult::Type::BOOL, "divisible", "whether the tokens are divisible"},
                    {RPCResult::Type::STR, "issuer", "the CounosH address of the issuer on record"},
                    {RPCResult::Type::STR_HEX, "creationtxid", "the hex-encoded creation transaction hash"},
                    {RPCResult::Type::BOOL, "fixedissuance", "whether the token supply is fixed"},
                    {RPCResult::Type::BOOL, "managedissuance", "whether the token supply is managed"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_listproperties", "")
           + HelpExampleRpc("counos_listproperties", "")
       }
    }.Check(request);

    UniValue response(UniValue::VARR);

    LOCK(cs_tally);

    uint32_t nextSPID = pDbSpInfo->peekNextSPID(1);
    for (uint32_t propertyId = 1; propertyId < nextSPID; propertyId++) {
        CMPSPInfo::Entry sp;
        if (pDbSpInfo->getSP(propertyId, sp)) {
            UniValue propertyObj(UniValue::VOBJ);
            propertyObj.pushKV("propertyid", (uint64_t) propertyId);
            PropertyToJSON(sp, propertyObj); // name, category, subcategory, ...

            response.push_back(propertyObj);
        }
    }

    uint32_t nextTestSPID = pDbSpInfo->peekNextSPID(2);
    for (uint32_t propertyId = TEST_ECO_PROPERTY_1; propertyId < nextTestSPID; propertyId++) {
        CMPSPInfo::Entry sp;
        if (pDbSpInfo->getSP(propertyId, sp)) {
            UniValue propertyObj(UniValue::VOBJ);
            propertyObj.pushKV("propertyid", (uint64_t) propertyId);
            PropertyToJSON(sp, propertyObj); // name, category, subcategory, ...

            response.push_back(propertyObj);
        }
    }

    return response;
}

static UniValue counos_getcrowdsale(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getcrowdsale",
       "\nReturns information about a crowdsale.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the crowdsale"},
           {"verbose", RPCArg::Type::BOOL, /* default */ "false", "list crowdsale participants"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "propertyid", "the identifier of the crowdsale"},
               {RPCResult::Type::STR, "name", "the name of the tokens issued via the crowdsale"},
               {RPCResult::Type::BOOL, "active", "whether the crowdsale is still active"},
               {RPCResult::Type::STR, "issuer", "the CounosH address of the issuer on record"},
               {RPCResult::Type::NUM, "propertyiddesired", "the identifier of the tokens eligible to participate in the crowdsale"},
               {RPCResult::Type::STR_AMOUNT, "tokensperunit", "the amount of tokens granted per unit invested in the crowdsale"},
               {RPCResult::Type::NUM, "earlybonus", "an early bird bonus for participants in percent per week"},
               {RPCResult::Type::NUM, "percenttoissuer", "a percentage of tokens that will be granted to the issuer"},
               {RPCResult::Type::NUM, "starttime", "the start time of the of the crowdsale as Unix timestamp"},
               {RPCResult::Type::NUM, "deadline", "the deadline of the crowdsale as Unix timestamp"},
               {RPCResult::Type::STR_AMOUNT, "amountraised", "the amount of tokens invested by participants"},
               {RPCResult::Type::STR_AMOUNT, "tokensissued", "the total number of tokens issued via the crowdsale"},
               {RPCResult::Type::STR_AMOUNT, "issuerbonustokens", "the amount of tokens granted to the issuer as bonus"},
               {RPCResult::Type::STR_AMOUNT, "addedissuertokens", "the amount of issuer bonus tokens not yet emitted"},
               {RPCResult::Type::BOOL, "closedearly", "whether the crowdsale ended early (if not active)"},
               {RPCResult::Type::BOOL, "maxtokens", "whether the crowdsale ended early due to reaching the limit of max. issuable tokens (if not active)"},
               {RPCResult::Type::NUM, "endedtime", "the time when the crowdsale ended (if closed early)"},
               {RPCResult::Type::STR_HEX, "closetx", "the hex-encoded hash of the transaction that closed the crowdsale (if closed manually)"},
               {RPCResult::Type::ARR, "participanttransactions", "",
               {
                    {RPCResult::Type::OBJ, "", "",
                    {
                        {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of participation transaction"},
                        {RPCResult::Type::STR_AMOUNT, "amountsent", "the amount of tokens invested by the participant"},
                        {RPCResult::Type::STR_AMOUNT, "participanttokens", "the tokens granted to the participant"},
                        {RPCResult::Type::STR_AMOUNT, "issuertokens",  "the tokens granted to the issuer as bonus"},
                   }},
               }},
           },
       },
       RPCExamples{
           HelpExampleCli("counos_getcrowdsale", "3 true")
           + HelpExampleRpc("counos_getcrowdsale", "3, true")
       }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);
    bool showVerbose = (request.params.size() > 1) ? request.params[1].get_bool() : false;

    RequireExistingProperty(propertyId);
    RequireCrowdsale(propertyId);

    CMPSPInfo::Entry sp;
    {
        LOCK(cs_tally);
        if (!pDbSpInfo->getSP(propertyId, sp)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Property identifier does not exist");
        }
    }

    const uint256& creationHash = sp.txid;

    bool f_txindex_ready = false;
    if (g_txindex) {
        f_txindex_ready = g_txindex->BlockUntilSyncedToCurrentChain();
    }

    CTransactionRef tx;
    uint256 hashBlock;
    if (!GetTransaction(creationHash, tx, Params().GetConsensus(), hashBlock)) {
        if (!f_txindex_ready) {
            PopulateFailure(MP_TXINDEX_STILL_SYNCING);
        } else {
            PopulateFailure(MP_TX_NOT_FOUND);
        }
    }

    UniValue response(UniValue::VOBJ);
    bool active = isCrowdsaleActive(propertyId);
    std::map<uint256, std::vector<int64_t> > database;

    if (active) {
        bool crowdFound = false;

        LOCK(cs_tally);

        for (CrowdMap::const_iterator it = my_crowds.begin(); it != my_crowds.end(); ++it) {
            const CMPCrowd& crowd = it->second;
            if (propertyId == crowd.getPropertyId()) {
                crowdFound = true;
                database = crowd.getDatabase();
            }
        }
        if (!crowdFound) {
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Crowdsale is flagged active but cannot be retrieved");
        }
    } else {
        database = sp.historicalData;
    }

    int64_t tokensIssued = getTotalTokens(propertyId);
    const std::string& txidClosed = sp.txid_close.GetHex();

    int64_t startTime = -1;
    if (!hashBlock.IsNull() && GetBlockIndex(hashBlock)) {
        startTime = GetBlockIndex(hashBlock)->nTime;
    }

    // note the database is already deserialized here and there is minimal performance penalty to iterate recipients to calculate amountRaised
    int64_t amountRaised = 0;
    int64_t amountIssuerTokens = 0;
    uint16_t propertyIdType = isPropertyDivisible(propertyId) ? MSC_PROPERTY_TYPE_DIVISIBLE : MSC_PROPERTY_TYPE_INDIVISIBLE;
    uint16_t desiredIdType = isPropertyDivisible(sp.property_desired) ? MSC_PROPERTY_TYPE_DIVISIBLE : MSC_PROPERTY_TYPE_INDIVISIBLE;
    std::map<std::string, UniValue> sortMap;
    for (std::map<uint256, std::vector<int64_t> >::const_iterator it = database.begin(); it != database.end(); it++) {
        UniValue participanttx(UniValue::VOBJ);
        std::string txid = it->first.GetHex();
        amountRaised += it->second.at(0);
        amountIssuerTokens += it->second.at(3);
        participanttx.pushKV("txid", txid);
        participanttx.pushKV("amountsent", FormatByType(it->second.at(0), desiredIdType));
        participanttx.pushKV("participanttokens", FormatByType(it->second.at(2), propertyIdType));
        participanttx.pushKV("issuertokens", FormatByType(it->second.at(3), propertyIdType));
        std::string sortKey = strprintf("%d-%s", it->second.at(1), txid);
        sortMap.insert(std::make_pair(sortKey, participanttx));
    }

    response.pushKV("propertyid", (uint64_t) propertyId);
    response.pushKV("name", sp.name);
    response.pushKV("active", active);
    response.pushKV("issuer", sp.issuer);
    response.pushKV("propertyiddesired", (uint64_t) sp.property_desired);
    response.pushKV("tokensperunit", FormatMP(propertyId, sp.num_tokens));
    response.pushKV("earlybonus", sp.early_bird);
    response.pushKV("percenttoissuer", sp.percentage);
    response.pushKV("starttime", startTime);
    response.pushKV("deadline", sp.deadline);
    response.pushKV("amountraised", FormatMP(sp.property_desired, amountRaised));
    response.pushKV("tokensissued", FormatMP(propertyId, tokensIssued));
    response.pushKV("issuerbonustokens", FormatMP(propertyId, amountIssuerTokens));
    response.pushKV("addedissuertokens", FormatMP(propertyId, sp.missedTokens));

    // TODO: return fields every time?
    if (!active) response.pushKV("closedearly", sp.close_early);
    if (!active) response.pushKV("maxtokens", sp.max_tokens);
    if (sp.close_early) response.pushKV("endedtime", sp.timeclosed);
    if (sp.close_early && !sp.max_tokens) response.pushKV("closetx", txidClosed);

    if (showVerbose) {
        UniValue participanttxs(UniValue::VARR);
        for (std::map<std::string, UniValue>::iterator it = sortMap.begin(); it != sortMap.end(); ++it) {
            participanttxs.push_back(it->second);
        }
        response.pushKV("participanttransactions", participanttxs);
    }

    return response;
}

static UniValue counos_getactivecrowdsales(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getactivecrowdsales",
       "\nLists currently active crowdsales.\n",
       {},
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "propertyid", "the identifier of the crowdsale"},
                   {RPCResult::Type::STR, "name", "the name of the tokens issued via the crowdsale"},
                   {RPCResult::Type::STR, "issuer", "the CounosH address of the issuer on record"},
                   {RPCResult::Type::NUM, "propertyiddesired", "the identifier of the tokens eligible to participate in the crowdsale"},
                   {RPCResult::Type::STR_AMOUNT, "tokensperunit", "the amount of tokens granted per unit invested in the crowdsale"},
                   {RPCResult::Type::NUM, "earlybonus", "an early bird bonus for participants in percent per week"},
                   {RPCResult::Type::NUM, "percenttoissuer", "a percentage of tokens that will be granted to the issuer"},
                   {RPCResult::Type::NUM, "starttime", "the start time of the of the crowdsale as Unix timestamp"},
                   {RPCResult::Type::NUM, "deadline", "the deadline of the crowdsale as Unix timestamp"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getactivecrowdsales", "")
           + HelpExampleRpc("counos_getactivecrowdsales", "")
       }
    }.Check(request);

    UniValue response(UniValue::VARR);

    LOCK2(cs_main, cs_tally);

    for (CrowdMap::const_iterator it = my_crowds.begin(); it != my_crowds.end(); ++it) {
        const CMPCrowd& crowd = it->second;
        uint32_t propertyId = crowd.getPropertyId();

        CMPSPInfo::Entry sp;
        if (!pDbSpInfo->getSP(propertyId, sp)) {
            continue;
        }

        const uint256& creationHash = sp.txid;

        bool f_txindex_ready = false;
        if (g_txindex) {
            f_txindex_ready = g_txindex->BlockUntilSyncedToCurrentChain();
        }

        CTransactionRef tx;
        uint256 hashBlock;
        if (!GetTransaction(creationHash, tx, Params().GetConsensus(), hashBlock)) {
            if (!f_txindex_ready) {
                PopulateFailure(MP_TXINDEX_STILL_SYNCING);
            } else {
                PopulateFailure(MP_TX_NOT_FOUND);
            }
        }

        int64_t startTime = -1;
        if (!hashBlock.IsNull() && GetBlockIndex(hashBlock)) {
            startTime = GetBlockIndex(hashBlock)->nTime;
        }

        UniValue responseObj(UniValue::VOBJ);
        responseObj.pushKV("propertyid", (uint64_t) propertyId);
        responseObj.pushKV("name", sp.name);
        responseObj.pushKV("issuer", sp.issuer);
        responseObj.pushKV("propertyiddesired", (uint64_t) sp.property_desired);
        responseObj.pushKV("tokensperunit", FormatMP(propertyId, sp.num_tokens));
        responseObj.pushKV("earlybonus", sp.early_bird);
        responseObj.pushKV("percenttoissuer", sp.percentage);
        responseObj.pushKV("starttime", startTime);
        responseObj.pushKV("deadline", sp.deadline);
        response.push_back(responseObj);
    }

    return response;
}

static UniValue counos_getgrants(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getgrants",
       "\nReturns information about granted and revoked units of managed tokens.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the identifier of the managed tokens to lookup"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "propertyid", "the identifier of the managed tokens"},
               {RPCResult::Type::STR, "name", "the name of the tokens"},
               {RPCResult::Type::STR, "issuer", "the CounosH address of the issuer on record"},
               {RPCResult::Type::STR_HEX, "creationtxid", "the hex-encoded creation transaction hash"},
               {RPCResult::Type::STR_AMOUNT, "totaltokens", "the total number of tokens in existence"},
               {RPCResult::Type::ARR, "issuances", "",
               {
                    {RPCResult::Type::OBJ, "", "",
                    {
                        {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction that granted tokens"},
                        {RPCResult::Type::STR_AMOUNT, "grant", "the number of tokens granted by this transaction"},
                    }},
                    {RPCResult::Type::OBJ, "", "",
                    {
                         {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction that revoked tokens"},
                         {RPCResult::Type::STR_AMOUNT, "grant", "the number of tokens revoked by this transaction"},
                    }},
               }},
           },
       },
       RPCExamples{
           HelpExampleCli("counos_getgrants", "31")
           + HelpExampleRpc("counos_getgrants", "31")
       }
    }.Check(request);

    uint32_t propertyId = ParsePropertyId(request.params[0]);

    RequireExistingProperty(propertyId);
    RequireManagedProperty(propertyId);

    CMPSPInfo::Entry sp;
    {
        LOCK(cs_tally);
        if (false == pDbSpInfo->getSP(propertyId, sp)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Property identifier does not exist");
        }
    }
    UniValue response(UniValue::VOBJ);
    const uint256& creationHash = sp.txid;
    int64_t totalTokens = getTotalTokens(propertyId);

    // TODO: sort by height?

    UniValue issuancetxs(UniValue::VARR);
    std::map<uint256, std::vector<int64_t> >::const_iterator it;
    for (it = sp.historicalData.begin(); it != sp.historicalData.end(); it++) {
        const std::string& txid = it->first.GetHex();
        int64_t grantedTokens = it->second.at(0);
        int64_t revokedTokens = it->second.at(1);

        if (grantedTokens > 0) {
            UniValue granttx(UniValue::VOBJ);
            granttx.pushKV("txid", txid);
            granttx.pushKV("grant", FormatMP(propertyId, grantedTokens));
            issuancetxs.push_back(granttx);
        }

        if (revokedTokens > 0) {
            UniValue revoketx(UniValue::VOBJ);
            revoketx.pushKV("txid", txid);
            revoketx.pushKV("revoke", FormatMP(propertyId, revokedTokens));
            issuancetxs.push_back(revoketx);
        }
    }

    response.pushKV("propertyid", (uint64_t) propertyId);
    response.pushKV("name", sp.name);
    response.pushKV("issuer", sp.issuer);
    response.pushKV("creationtxid", creationHash.GetHex());
    response.pushKV("totaltokens", FormatMP(propertyId, totalTokens));
    response.pushKV("issuances", issuancetxs);

    return response;
}

static UniValue counos_getorderbook(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getorderbook",
       "\nList active offers on the distributed token exchange.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "filter orders by property identifier for sale"},
           {"propertyiddesired", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "filter orders by property identifier desired"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                    {RPCResult::Type::STR, "address", "the CounosH address of the trader"},
                    {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction of the order"},
                    {RPCResult::Type::STR, "ecosystem", "the ecosytem in which the order was made (if \"cancel-ecosystem\")"},
                    {RPCResult::Type::NUM, "propertyidforsale", "the identifier of the tokens put up for sale"},
                    {RPCResult::Type::BOOL, "propertyidforsaleisdivisible", "whether the tokens for sale are divisible"},
                    {RPCResult::Type::STR_AMOUNT, "amountforsale", "the amount of tokens initially offered"},
                    {RPCResult::Type::STR_AMOUNT, "amountremaining", "the amount of tokens still up for sale"},
                    {RPCResult::Type::NUM, "propertyiddesired", "the identifier of the tokens desired in exchange"},
                    {RPCResult::Type::BOOL, "propertyiddesiredisdivisible", "whether the desired tokens are divisible"},
                    {RPCResult::Type::STR_AMOUNT, "amountdesired", "the amount of tokens initially desired"},
                    {RPCResult::Type::STR_AMOUNT, "amounttofill", "the amount of tokens still needed to fill the offer completely"},
                    {RPCResult::Type::NUM, "action", "the action of the transaction: (1) \"trade\", (2) \"cancel-price\", (3) \"cancel-pair\", (4) \"cancel-ecosystem\""},
                    {RPCResult::Type::NUM, "block", "the index of the block that contains the transaction"},
                    {RPCResult::Type::NUM, "blocktime", "the timestamp of the block that contains the transaction"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getorderbook", "2")
           + HelpExampleRpc("counos_getorderbook", "2")
       }
    }.Check(request);

    bool filterDesired = (request.params.size() > 1);
    uint32_t propertyIdForSale = ParsePropertyId(request.params[0]);
    uint32_t propertyIdDesired = 0;

    RequireExistingProperty(propertyIdForSale);

    if (filterDesired) {
        propertyIdDesired = ParsePropertyId(request.params[1]);

        RequireExistingProperty(propertyIdDesired);
        RequireSameEcosystem(propertyIdForSale, propertyIdDesired);
        RequireDifferentIds(propertyIdForSale, propertyIdDesired);
    }

    std::vector<CMPMetaDEx> vecMetaDexObjects;
    {
        LOCK(cs_tally);
        for (md_PropertiesMap::const_iterator my_it = metadex.begin(); my_it != metadex.end(); ++my_it) {
            const md_PricesMap& prices = my_it->second;
            for (md_PricesMap::const_iterator it = prices.begin(); it != prices.end(); ++it) {
                const md_Set& indexes = it->second;
                for (md_Set::const_iterator it = indexes.begin(); it != indexes.end(); ++it) {
                    const CMPMetaDEx& obj = *it;
                    if (obj.getProperty() != propertyIdForSale) continue;
                    if (!filterDesired || obj.getDesProperty() == propertyIdDesired) vecMetaDexObjects.push_back(obj);
                }
            }
        }
    }

    UniValue response(UniValue::VARR);
    MetaDexObjectsToJSON(vecMetaDexObjects, response);
    return response;
}

static UniValue counos_gettradehistoryforaddress(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);
#else
    std::unique_ptr<interfaces::Wallet> pWallet;
#endif

    RPCHelpMan{"counos_gettradehistoryforaddress",
       "\nRetrieves the history of orders on the distributed exchange for the supplied address.\n"
       "\nNote:\n"
       "The documentation only covers the output for a trade, but there are also cancel transactions with different properties.\n",
       {
           {"address", RPCArg::Type::STR, RPCArg::Optional::NO, "address to retrieve history for"},
           {"count", RPCArg::Type::NUM, /* default */ "10", "number of orders to retrieve"},
           {"propertyid", RPCArg::Type::NUM, /* default */ "no filter", "filter by property identifier transacted"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction of the order"},
                   {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the trader"},
                   {RPCResult::Type::BOOL, "ismine", "whether the order involes an address in the wallet"},
                   {RPCResult::Type::NUM, "confirmations", "the number of transaction confirmations"},
                   {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
                   {RPCResult::Type::NUM, "blocktime", "the timestamp of the block that contains the transaction"},
                   {RPCResult::Type::BOOL, "valid", "whether the transaction is valid"},
                   {RPCResult::Type::NUM, "version", "the transaction version"},
                   {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
                   {RPCResult::Type::STR, "type", "the transaction type as string"},
                   {RPCResult::Type::NUM, "propertyidforsale", "the identifier of the tokens put up for sale"},
                   {RPCResult::Type::BOOL, "propertyidforsaleisdivisible", "whether the tokens for sale are divisible"},
                   {RPCResult::Type::STR_AMOUNT, "amountforsale", "the amount of tokens initially offered"},
                   {RPCResult::Type::NUM, "propertyiddesired", "the identifier of the tokens desired in exchange"},
                   {RPCResult::Type::BOOL, "propertyiddesiredisdivisible", "whether the desired tokens are divisible"},
                   {RPCResult::Type::STR_AMOUNT, "amountdesired", "the amount of tokens initially desired"},
                   {RPCResult::Type::STR_AMOUNT, "unitprice", "the unit price (shown in the property desired)"},
                   {RPCResult::Type::STR, "status" , "the status of the order (\"open\", \"cancelled\", \"filled\", ...)"},
                   {RPCResult::Type::STR_HEX, "canceltxid", "the hash of the transaction that cancelled the order (if cancelled)"},
                   {RPCResult::Type::ARR, "matches", "a list of matched orders and executed trades",
                   {
                       {RPCResult::Type::OBJ, "", "",
                       {
                           {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction that was matched against"},
                           {RPCResult::Type::NUM, "block", "the index of the block that contains this transaction"},
                           {RPCResult::Type::STR, "address", "the CounosH address of the other trader"},
                           {RPCResult::Type::STR_AMOUNT, "amountsold", "the number of tokens sold in this trade"},
                           {RPCResult::Type::STR_AMOUNT, "amountreceived", "the number of tokens traded in exchange"},
                       }},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_gettradehistoryforaddress", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\"")
           + HelpExampleRpc("counos_gettradehistoryforaddress", "\"cch1qltvlx8t4cfa2jg8g6gjze30jqtdsu9jf3pegpj\"")
       }
    }.Check(request);

    std::string address = ParseAddress(request.params[0]);
    uint64_t count = (request.params.size() > 1) ? request.params[1].get_int64() : 10;
    uint32_t propertyId = 0;

    if (request.params.size() > 2) {
        propertyId = ParsePropertyId(request.params[2]);
        RequireExistingProperty(propertyId);
    }

    // Obtain a sorted vector of txids for the address trade history
    std::vector<uint256> vecTransactions;
    {
        LOCK(cs_tally);
        pDbTradeList->getTradesForAddress(address, vecTransactions, propertyId);
    }

    // Populate the address trade history into JSON objects until we have processed count transactions
    UniValue response(UniValue::VARR);
    uint32_t processed = 0;
    for(std::vector<uint256>::reverse_iterator it = vecTransactions.rbegin(); it != vecTransactions.rend(); ++it) {
        UniValue txobj(UniValue::VOBJ);
        int populateResult = populateRPCTransactionObject(*it, txobj, "", true, "", pWallet.get());
        if (0 == populateResult) {
            response.push_back(txobj);
            processed++;
            if (processed >= count) break;
        }
    }

    return response;
}

static UniValue counos_gettradehistoryforpair(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_gettradehistoryforpair",
       "\nRetrieves the history of trades on the distributed token exchange for the specified market.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the first side of the traded pair"},
           {"propertyidsecond", RPCArg::Type::NUM, RPCArg::Optional::NO, "the second side of the traded pair"},
           {"count", RPCArg::Type::NUM, /* default */ "10", "number of trades to retrieve"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::NUM, "block", "the index of the block that contains the trade match"},
                   {RPCResult::Type::STR_AMOUNT, "unitprice", "the unit price used to execute this trade (received/sold)"},
                   {RPCResult::Type::STR_AMOUNT, "inverseprice", "the inverse unit price (sold/received)"},
                   {RPCResult::Type::STR_HEX, "sellertxid", "the hash of the transaction of the seller"},
                   {RPCResult::Type::STR, "address", "the CounosH address of the seller"},
                   {RPCResult::Type::STR_AMOUNT, "amountsold", "the number of tokens sold in this trade"},
                   {RPCResult::Type::STR_AMOUNT, "amountreceived", "the number of tokens traded in exchange"},
                   {RPCResult::Type::STR_HEX, "matchingtxid", "the hash of the transaction that was matched against"},
                   {RPCResult::Type::STR, "matchingaddress", "the CounosH address of the other party of this trade"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_gettradehistoryforpair", "1 12 500")
           + HelpExampleRpc("counos_gettradehistoryforpair", "1, 12, 500")
       }
    }.Check(request);

    // obtain property identifiers for pair & check valid parameters
    uint32_t propertyIdSideA = ParsePropertyId(request.params[0]);
    uint32_t propertyIdSideB = ParsePropertyId(request.params[1]);
    uint64_t count = (request.params.size() > 2) ? request.params[2].get_int64() : 10;

    RequireExistingProperty(propertyIdSideA);
    RequireExistingProperty(propertyIdSideB);
    RequireSameEcosystem(propertyIdSideA, propertyIdSideB);
    RequireDifferentIds(propertyIdSideA, propertyIdSideB);

    // request pair trade history from trade db
    UniValue response(UniValue::VARR);
    LOCK(cs_tally);
    pDbTradeList->getTradesForPair(propertyIdSideA, propertyIdSideB, response, count);
    return response;
}

static UniValue counos_getactivedexsells(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getactivedexsells",
       "\nReturns currently active offers on the distributed exchange.\n",
       {
           {"address", RPCArg::Type::STR, /* default */ "include any", "address filter"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction of this offer"},
                   {RPCResult::Type::NUM, "propertyid", "the identifier of the tokens for sale"},
                   {RPCResult::Type::STR, "seller", "the CounosH address of the seller"},
                   {RPCResult::Type::STR_AMOUNT, "amountavailable", "the number of tokens still listed for sale and currently available"},
                   {RPCResult::Type::STR_AMOUNT, "counoshdesired", "the number of counoshs desired in exchange"},
                   {RPCResult::Type::STR_AMOUNT, "unitprice", "the unit price (CCH/token)"},
                   {RPCResult::Type::NUM, "timelimit", "the time limit in blocks a buyer has to pay following a successful accept"},
                   {RPCResult::Type::STR_AMOUNT, "minimumfee", "the minimum mining fee a buyer has to pay to accept this offer"},
                   {RPCResult::Type::STR_AMOUNT, "amountaccepted", "the number of tokens currently reserved for pending \"accept\" orders"},
                   {RPCResult::Type::ARR, "accepts", "a list of pending \"accept\" orders",
                   {
                       {RPCResult::Type::OBJ, "", "",
                       {
                           {RPCResult::Type::STR, "buyer", "the CounosH address of the buyer"},
                           {RPCResult::Type::NUM, "block", "the index of the block that contains the \"accept\" order"},
                           {RPCResult::Type::NUM, "blocksleft", "the number of blocks left to pay"},
                           {RPCResult::Type::STR_AMOUNT, "amount", "the amount of tokens accepted and reserved"},
                           {RPCResult::Type::STR_AMOUNT, "amounttopay", "the amount in counoshs needed finalize the trade"},
                       }},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getactivedexsells", "")
           + HelpExampleRpc("counos_getactivedexsells", "")
       }
    }.Check(request);

    std::string addressFilter;

    if (request.params.size() > 0) {
        addressFilter = ParseAddressOrEmpty(request.params[0]);
    }

    UniValue response(UniValue::VARR);

    int curBlock = GetHeight();

    LOCK(cs_tally);

    for (OfferMap::iterator it = my_offers.begin(); it != my_offers.end(); ++it) {
        const CMPOffer& selloffer = it->second;
        std::vector<std::string> vstr;
        boost::split(vstr, it->first, boost::is_any_of("-"), boost::token_compress_on);
        std::string seller = vstr[0];

        // filtering
        if (!addressFilter.empty() && seller != addressFilter) continue;

        std::string txid = selloffer.getHash().GetHex();
        uint32_t propertyId = selloffer.getProperty();
        int64_t minFee = selloffer.getMinFee();
        uint8_t timeLimit = selloffer.getBlockTimeLimit();
        int64_t sellOfferAmount = selloffer.getOfferAmountOriginal(); //badly named - "Original" implies off the wire, but is amended amount
        int64_t sellCounosHDesired = selloffer.getCCHDesiredOriginal(); //badly named - "Original" implies off the wire, but is amended amount
        int64_t amountAvailable = GetTokenBalance(seller, propertyId, SELLOFFER_RESERVE);
        int64_t amountAccepted = GetTokenBalance(seller, propertyId, ACCEPT_RESERVE);

        // TODO: no math, and especially no rounding here (!)
        // TODO: no math, and especially no rounding here (!)
        // TODO: no math, and especially no rounding here (!)

        // calculate unit price and updated amount of counosh desired
        double unitPriceFloat = 0.0;
        if ((sellOfferAmount > 0) && (sellCounosHDesired > 0)) {
            unitPriceFloat = (double) sellCounosHDesired / (double) sellOfferAmount; // divide by zero protection
            if (!isPropertyDivisible(propertyId)) {
                unitPriceFloat /= 100000000.0;
            }
        }
        int64_t unitPrice = rounduint64(unitPriceFloat * COIN);
        int64_t counoshDesired = calculateDesiredCCH(sellOfferAmount, sellCounosHDesired, amountAvailable);

        UniValue responseObj(UniValue::VOBJ);
        responseObj.pushKV("txid", txid);
        responseObj.pushKV("propertyid", (uint64_t) propertyId);
        responseObj.pushKV("seller", seller);
        responseObj.pushKV("amountavailable", FormatMP(propertyId, amountAvailable));
        responseObj.pushKV("counoshdesired", FormatDivisibleMP(counoshDesired));
        responseObj.pushKV("unitprice", FormatDivisibleMP(unitPrice));
        responseObj.pushKV("timelimit", timeLimit);
        responseObj.pushKV("minimumfee", FormatDivisibleMP(minFee));

        // display info about accepts related to sell
        responseObj.pushKV("amountaccepted", FormatMP(propertyId, amountAccepted));
        UniValue acceptsMatched(UniValue::VARR);
        for (AcceptMap::const_iterator ait = my_accepts.begin(); ait != my_accepts.end(); ++ait) {
            UniValue matchedAccept(UniValue::VOBJ);
            const CMPAccept& accept = ait->second;
            const std::string& acceptCombo = ait->first;

            // does this accept match the sell?
            if (accept.getHash() == selloffer.getHash()) {
                // split acceptCombo out to get the buyer address
                std::string buyer = acceptCombo.substr((acceptCombo.find("+") + 1), (acceptCombo.size()-(acceptCombo.find("+") + 1)));
                int blockOfAccept = accept.getAcceptBlock();
                int blocksLeftToPay = (blockOfAccept + selloffer.getBlockTimeLimit()) - curBlock;
                int64_t amountAccepted = accept.getAcceptAmountRemaining();
                // TODO: don't recalculate!
                int64_t amountToPayInCCH = calculateDesiredCCH(accept.getOfferAmountOriginal(), accept.getCCHDesiredOriginal(), amountAccepted);
                matchedAccept.pushKV("buyer", buyer);
                matchedAccept.pushKV("block", blockOfAccept);
                matchedAccept.pushKV("blocksleft", blocksLeftToPay);
                matchedAccept.pushKV("amount", FormatMP(propertyId, amountAccepted));
                matchedAccept.pushKV("amounttopay", FormatDivisibleMP(amountToPayInCCH));
                acceptsMatched.push_back(matchedAccept);
            }
        }
        responseObj.pushKV("accepts", acceptsMatched);

        // add sell object into response array
        response.push_back(responseObj);
    }

    return response;
}

static UniValue counos_listblocktransactions(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_listblocktransactions",
       "\nLists all Counos transactions in a block.\n",
       {
           {"index", RPCArg::Type::NUM, RPCArg::Optional::NO, "the block height or block index"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction"},
           }
       },
       RPCExamples{
        HelpExampleCli("counos_listblocktransactions", "279007")
        + HelpExampleRpc("counos_listblocktransactions", "279007")
       }
    }.Check(request);

    int blockHeight = request.params[0].get_int();

    RequireHeightInChain(blockHeight);

    // next let's obtain the block for this height
    CBlock block;
    {
        LOCK(cs_main);
        CBlockIndex* pBlockIndex = ::ChainActive()[blockHeight];

        if (!ReadBlockFromDisk(block, pBlockIndex, Params().GetConsensus())) {
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Failed to read block from disk");
        }
    }

    UniValue response(UniValue::VARR);

    // now we want to loop through each of the transactions in the block and run against CMPTxList::exists
    // those that return positive add to our response array

    LOCK(cs_tally);

    for(const auto tx : block.vtx) {
        if (pDbTransactionList->exists(tx->GetHash())) {
            // later we can add a verbose flag to decode here, but for now callers can send returned txids into gettransaction_MP
            // add the txid into the response as it's an MP transaction
            response.push_back(tx->GetHash().GetHex());
        }
    }

    return response;
}

static UniValue counos_listblockstransactions(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_listblocktransactions",
       "\nLists all Counos transactions in a given range of blocks.\n",
       {
           {"firstblock", RPCArg::Type::NUM, RPCArg::Optional::NO, "the index of the first block to consider"},
           {"lastblock", RPCArg::Type::NUM, RPCArg::Optional::NO, "the index of the last block to consider"},
       },
       RPCResult{
                   RPCResult::Type::ARR, "", "",
                   {
                       {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction"},
                   }
       },
       RPCExamples{
           HelpExampleCli("counos_listblocktransactions", "279007 300000")
           + HelpExampleRpc("counos_listblocktransactions", "279007, 300000")
       }
    }.Check(request);

    int blockFirst = request.params[0].get_int();
    int blockLast = request.params[1].get_int();

    std::set<uint256> txs;
    UniValue response(UniValue::VARR);

    LOCK(cs_tally);
    {
        pDbTransactionList->GetCounosTxsInBlockRange(blockFirst, blockLast, txs);
    }

    for(const uint256& tx : txs) {
        response.push_back(tx.GetHex());
    }

    return response;
}

static UniValue counos_gettransaction(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);
#else
    std::unique_ptr<interfaces::Wallet> pWallet;
#endif

    RPCHelpMan{"counos_gettransaction",
       "\nGet detailed information about an Counos transaction.\n",
       {
           {"txid", RPCArg::Type::STR, RPCArg::Optional::NO, "the hash of the transaction to lookup"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction"},
               {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the sender"},
               {RPCResult::Type::STR, "referenceaddress", "a CounosH address used as reference (if any)"},
               {RPCResult::Type::BOOL, "ismine", "whether the transaction involes an address in the wallet"},
               {RPCResult::Type::NUM, "confirmations", "the number of transaction confirmations"},
               {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
               {RPCResult::Type::STR_AMOUNT, "blocktime", "the timestamp of the block that contains the transaction"},
               {RPCResult::Type::BOOL, "valid", "whether the transaction is valid"},
               {RPCResult::Type::STR, "invalidreason", "if a transaction is invalid, the reason"},
               {RPCResult::Type::NUM, "version", "the transaction version"},
               {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
               {RPCResult::Type::STR, "type", "the transaction type as string"},
               {RPCResult::Type::ELISION, "", "other transaction type specific properties"},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_gettransaction", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
           + HelpExampleRpc("counos_gettransaction", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
       }
    }.Check(request);

    uint256 hash = ParseHashV(request.params[0], "txid");

    UniValue txobj(UniValue::VOBJ);
    int populateResult = populateRPCTransactionObject(hash, txobj, "", false, "", pWallet.get());
    if (populateResult != 0) PopulateFailure(populateResult);

    return txobj;
}

#ifdef ENABLE_WALLET
static UniValue counos_listtransactions(const JSONRPCRequest& request)
{
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);

    RPCHelpMan{"counos_listtransactions",
       "\nList wallet transactions, optionally filtered by an address and block boundaries.\n",
       {
           {"address", RPCArg::Type::STR, /* default */ "\"*\"", "address filter"},
           {"count", RPCArg::Type::NUM, /* default */ "10", "show at most n transactions"},
           {"skip", RPCArg::Type::NUM, /* default */ "0", "skip the first n transactions"},
           {"startblock", RPCArg::Type::NUM, /* default */ "0", "first block to begin the search"},
           {"endblock", RPCArg::Type::NUM, /* default */ "999999999", "last block to include in the search"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction"},
                   {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the sender"},
                   {RPCResult::Type::STR, "referenceaddress", "a CounosH address used as reference (if any)"},
                   {RPCResult::Type::BOOL, "ismine", "whether the transaction involes an address in the wallet"},
                   {RPCResult::Type::NUM, "confirmations", "the number of transaction confirmations"},
                   {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
                   {RPCResult::Type::STR_AMOUNT, "blocktime", "the timestamp of the block that contains the transaction"},
                   {RPCResult::Type::BOOL, "valid", "whether the transaction is valid"},
                   {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
                   {RPCResult::Type::STR, "type", "the transaction type as string"},
                   {RPCResult::Type::ELISION, "", "other transaction type specific properties"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_listtransactions", "")
           + HelpExampleRpc("counos_listtransactions", "")
       }
    }.Check(request);

    // obtains parameters - default all wallet addresses & last 10 transactions
    std::string addressParam;
    if (request.params.size() > 0) {
        if (("*" != request.params[0].get_str()) && ("" != request.params[0].get_str())) addressParam = request.params[0].get_str();
    }
    int64_t nCount = 10;
    if (request.params.size() > 1) nCount = request.params[1].get_int64();
    if (nCount < 0) throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    int64_t nFrom = 0;
    if (request.params.size() > 2) nFrom = request.params[2].get_int64();
    if (nFrom < 0) throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");
    int64_t nStartBlock = 0;
    if (request.params.size() > 3) nStartBlock = request.params[3].get_int64();
    if (nStartBlock < 0) throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative start block");
    int64_t nEndBlock = 999999999;
    if (request.params.size() > 4) nEndBlock = request.params[4].get_int64();
    if (nEndBlock < 0) throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative end block");

    // obtain a sorted list of Counos layer wallet transactions (including STO receipts and pending)
    std::map<std::string,uint256> walletTransactions = FetchWalletCounosTransactions(*pWallet, nFrom+nCount, nStartBlock, nEndBlock);

    // reverse iterate over (now ordered) transactions and populate RPC objects for each one
    UniValue response(UniValue::VARR);
    for (std::map<std::string,uint256>::reverse_iterator it = walletTransactions.rbegin(); it != walletTransactions.rend(); it++) {
        if (nFrom <= 0 && nCount > 0) {
            uint256 txHash = it->second;
            UniValue txobj(UniValue::VOBJ);
            int populateResult = populateRPCTransactionObject(txHash, txobj, addressParam, false, "", pWallet.get());
            if (0 == populateResult) {
                response.push_back(txobj);
                nCount--;
            }
        }
        nFrom--;
    }

    return response;
}
#endif

static UniValue counos_listpendingtransactions(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);
#else
    std::unique_ptr<interfaces::Wallet> pWallet;
#endif

    RPCHelpMan{"counos_listpendingtransactions",
       "\nReturns a list of unconfirmed Counos transactions, pending in the memory pool.\n"
       "\nAn optional filter can be provided to only include transactions which involve the given address.\n"
       "\nNote: the validity of pending transactions is uncertain, and the state of the memory pool may "
       "change at any moment. It is recommended to check transactions after confirmation, and pending "
       "transactions should be considered as invalid.\n",
       {
           {"address", RPCArg::Type::STR, /* default */ "\"\" for no filter", "address filter"},
       },
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::OBJ, "", "",
               {
                   {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction"},
                   {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the sender"},
                   {RPCResult::Type::STR, "referenceaddress", "a CounosH address used as reference (if any)"},
                   {RPCResult::Type::BOOL, "ismine", "whether the transaction involes an address in the wallet"},
                   {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
                   {RPCResult::Type::NUM, "version", "the transaction version"},
                   {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
                   {RPCResult::Type::STR, "type", "the transaction type as string"},
                   {RPCResult::Type::ELISION, "", "other transaction type specific properties"},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_listpendingtransactions", "")
           + HelpExampleRpc("counos_listpendingtransactions", "")
       }
    }.Check(request);

    std::string filterAddress;
    if (request.params.size() > 0) {
        filterAddress = ParseAddressOrEmpty(request.params[0]);
    }

    std::vector<uint256> vTxid;
    mempool.queryHashes(vTxid);

    UniValue result(UniValue::VARR);
    for(const uint256& hash : vTxid) {
        if (!IsInMarkerCache(hash)) {
            continue;
        }

        UniValue txObj(UniValue::VOBJ);
        if (populateRPCTransactionObject(hash, txObj, filterAddress, false, "", pWallet.get()) == 0) {
            result.push_back(txObj);
        }
    }

    return result;
}

static UniValue counos_getinfo(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getinfo",
       "Returns various state information of the client and protocol.\n",
       {},
       RPCResult{
           RPCResult::Type::ARR, "", "",
           {
               {RPCResult::Type::NUM, "counoscoreversion_int", "client version as integer"},
               {RPCResult::Type::STR, "counoscoreversion", "client version"},
               {RPCResult::Type::STR, "mastercoreversion", "client version (DEPRECATED)"},
               {RPCResult::Type::STR, "counoshcoreversion", "CounosH Core version"},
               {RPCResult::Type::NUM, "block", "index of the last processed block"},
               {RPCResult::Type::NUM, "blocktime", "timestamp of the last processed block"},
               {RPCResult::Type::NUM, "blocktransactions", "Counos transactions found in the last processed block"},
               {RPCResult::Type::NUM, "totaltransactions", "Counos transactions processed in total"},
               {RPCResult::Type::ARR, "alerts", "active protocol alert (if any)",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::NUM, "alerttypeint", "alert type as integer"},
                       {RPCResult::Type::STR, "alerttype", "alert type"},
                       {RPCResult::Type::STR, "alertexpiry", "expiration criteria"},
                       {RPCResult::Type::STR, "alertmessage", "information about the alert"},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getinfo", "")
           + HelpExampleRpc("counos_getinfo", "")
       }
    }.Check(request);

    UniValue infoResponse(UniValue::VOBJ);

    // provide the mastercore and counosh version
    infoResponse.pushKV("counoscoreversion_int", COUNOSCORE_VERSION);
    infoResponse.pushKV("counoscoreversion", CounosCoreVersion());
    infoResponse.pushKV("mastercoreversion", CounosCoreVersion());
    infoResponse.pushKV("counoshcoreversion", CounosHCoreVersion());

    // provide the current block details
    int block = GetHeight();
    int64_t blockTime = GetLatestBlockTime();

    LOCK(cs_tally);

    int blockMPTransactions = pDbTransactionList->getMPTransactionCountBlock(block);
    int totalMPTransactions = pDbTransactionList->getMPTransactionCountTotal();
    int totalMPTrades = pDbTradeList->getMPTradeCountTotal();
    infoResponse.pushKV("block", block);
    infoResponse.pushKV("blocktime", blockTime);
    infoResponse.pushKV("blocktransactions", blockMPTransactions);

    // provide the number of trades completed
    infoResponse.pushKV("totaltrades", totalMPTrades);
    // provide the number of transactions parsed
    infoResponse.pushKV("totaltransactions", totalMPTransactions);

    // handle alerts
    UniValue alerts(UniValue::VARR);
    std::vector<AlertData> counosAlerts = GetCounosCoreAlerts();
    for (std::vector<AlertData>::iterator it = counosAlerts.begin(); it != counosAlerts.end(); it++) {
        AlertData alert = *it;
        UniValue alertResponse(UniValue::VOBJ);
        std::string alertTypeStr;
        switch (alert.alert_type) {
            case 1: alertTypeStr = "alertexpiringbyblock";
            break;
            case 2: alertTypeStr = "alertexpiringbyblocktime";
            break;
            case 3: alertTypeStr = "alertexpiringbyclientversion";
            break;
            default: alertTypeStr = "error";
        }
        alertResponse.pushKV("alerttypeint", alert.alert_type);
        alertResponse.pushKV("alerttype", alertTypeStr);
        alertResponse.pushKV("alertexpiry", FormatIndivisibleMP(alert.alert_expiry));
        alertResponse.pushKV("alertmessage", alert.alert_message);
        alerts.push_back(alertResponse);
    }
    infoResponse.pushKV("alerts", alerts);

    return infoResponse;
}

static UniValue counos_getactivations(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getactivations",
       "Returns pending and completed feature activations.\n",
       {},
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::ARR, "pendingactivations", "a list of pending feature activations",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::NUM, "featureid", "the id of the feature"},
                       {RPCResult::Type::STR, "featurename", "the name of the feature"},
                       {RPCResult::Type::NUM, "activationblock", "the block the feature will be activated"},
                       {RPCResult::Type::NUM, "minimumversion", "the minimum client version needed to support this feature"},
                   }},
               }},
               {RPCResult::Type::ARR, "completedactivations", "a list of completed feature activations",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::NUM, "featureid", "the id of the feature"},
                       {RPCResult::Type::STR, "featurename", "the name of the feature"},
                       {RPCResult::Type::NUM, "activationblock", "the block the feature will be activated"},
                       {RPCResult::Type::NUM, "minimumversion", "the minimum client version needed to support this feature"},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getactivations", "")
           + HelpExampleRpc("counos_getactivations", "")
       }
    }.Check(request);

    UniValue response(UniValue::VOBJ);

    UniValue arrayPendingActivations(UniValue::VARR);
    std::vector<FeatureActivation> vecPendingActivations = GetPendingActivations();
    for (std::vector<FeatureActivation>::iterator it = vecPendingActivations.begin(); it != vecPendingActivations.end(); ++it) {
        UniValue actObj(UniValue::VOBJ);
        FeatureActivation pendingAct = *it;
        actObj.pushKV("featureid", pendingAct.featureId);
        actObj.pushKV("featurename", pendingAct.featureName);
        actObj.pushKV("activationblock", pendingAct.activationBlock);
        actObj.pushKV("minimumversion", (uint64_t)pendingAct.minClientVersion);
        arrayPendingActivations.push_back(actObj);
    }

    UniValue arrayCompletedActivations(UniValue::VARR);
    std::vector<FeatureActivation> vecCompletedActivations = GetCompletedActivations();
    for (std::vector<FeatureActivation>::iterator it = vecCompletedActivations.begin(); it != vecCompletedActivations.end(); ++it) {
        UniValue actObj(UniValue::VOBJ);
        FeatureActivation completedAct = *it;
        actObj.pushKV("featureid", completedAct.featureId);
        actObj.pushKV("featurename", completedAct.featureName);
        actObj.pushKV("activationblock", completedAct.activationBlock);
        actObj.pushKV("minimumversion", (uint64_t)completedAct.minClientVersion);
        arrayCompletedActivations.push_back(actObj);
    }

    response.pushKV("pendingactivations", arrayPendingActivations);
    response.pushKV("completedactivations", arrayCompletedActivations);

    return response;
}

static UniValue counos_getsto(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);
#else
    std::unique_ptr<interfaces::Wallet> pWallet;
#endif

    RPCHelpMan{"counos_getsto",
       "\nGet information and recipients of a send-to-owners transaction.\n",
       {
           {"txid", RPCArg::Type::STR, RPCArg::Optional::NO, "the hash of the transaction to lookup"},
           {"recipientfilter", RPCArg::Type::STR, /* default */ "\"*\" for all", "a filter for recipients"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction"},
               {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the sender"},
               {RPCResult::Type::BOOL, "ismine", "whether the transaction involes an address in the wallet"},
               {RPCResult::Type::NUM, "confirmations", "the number of transaction confirmations"},
               {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
               {RPCResult::Type::STR_AMOUNT, "blocktime", "the timestamp of the block that contains the transaction"},
               {RPCResult::Type::BOOL, "valid", "whether the transaction is valid"},
               {RPCResult::Type::NUM, "version", "the transaction version"},
               {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
               {RPCResult::Type::STR, "type", "the transaction type as string"},
               {RPCResult::Type::NUM, "propertyid", "the identifier of sent tokens"},
               {RPCResult::Type::BOOL, "divisible", "whether the sent tokens are divisible"},
               {RPCResult::Type::STR_AMOUNT, "amount", "the number of tokens sent to owners"},
               {RPCResult::Type::STR_AMOUNT, "totalstofee", "the fee paid by the sender, nominated in COUN or TCOUN"},
               {RPCResult::Type::ARR, "recipients", "a list of recipients",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::STR, "address", "the CounosH address of the recipient"},
                       {RPCResult::Type::STR_AMOUNT, "amount", "the number of tokens sent to this recipient"},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getsto", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\" \"*\"")
           + HelpExampleRpc("counos_getsto", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\", \"*\"")
       }
    }.Check(request);

    uint256 hash = ParseHashV(request.params[0], "txid");
    std::string filterAddress;
    if (request.params.size() > 1) filterAddress = ParseAddressOrWildcard(request.params[1]);

    UniValue txobj(UniValue::VOBJ);
    int populateResult = populateRPCTransactionObject(hash, txobj, "", true, filterAddress, pWallet.get());
    if (populateResult != 0) PopulateFailure(populateResult);

    return txobj;
}

static UniValue counos_gettrade(const JSONRPCRequest& request)
{
#ifdef ENABLE_WALLET
    std::shared_ptr<CWallet> const wallet = GetWalletForJSONRPCRequest(request);
    std::unique_ptr<interfaces::Wallet> pWallet = interfaces::MakeWallet(wallet);
#else
    std::unique_ptr<interfaces::Wallet> pWallet;
#endif

    RPCHelpMan{"counos_gettrade",
       "\nGet detailed information and trade matches for orders on the distributed token exchange.\n"
       "\nNote:\n"
       "The documentation only covers the output for a trade, but there are also cancel transactions with different properties.\n",
       {
           {"txid", RPCArg::Type::STR, RPCArg::Optional::NO, "the hash of the order to lookup"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::STR_HEX, "txid", "the hex-encoded hash of the transaction of the order"},
               {RPCResult::Type::STR, "sendingaddress", "the CounosH address of the trader"},
               {RPCResult::Type::BOOL, "ismine", "whether the order involes an address in the wallet"},
               {RPCResult::Type::NUM, "confirmations", "the number of transaction confirmations"},
               {RPCResult::Type::STR_AMOUNT, "fee", "the transaction fee in counoshs"},
               {RPCResult::Type::NUM, "blocktime", "the timestamp of the block that contains the transaction"},
               {RPCResult::Type::BOOL, "valid", "whether the transaction is valid"},
               {RPCResult::Type::NUM, "version", "the transaction version"},
               {RPCResult::Type::NUM, "type_int", "the transaction type as number"},
               {RPCResult::Type::STR, "type", "the transaction type as string"},
               {RPCResult::Type::NUM, "propertyidforsale", "the identifier of the tokens put up for sale"},
               {RPCResult::Type::BOOL, "propertyidforsaleisdivisible", "whether the tokens for sale are divisible"},
               {RPCResult::Type::STR_AMOUNT, "amountforsale", "the amount of tokens initially offered"},
               {RPCResult::Type::NUM, "propertyiddesired", "the identifier of the tokens desired in exchange"},
               {RPCResult::Type::BOOL, "propertyiddesiredisdivisible", "whether the desired tokens are divisible"},
               {RPCResult::Type::STR_AMOUNT, "amountdesired", "the amount of tokens initially desired"},
               {RPCResult::Type::STR_AMOUNT, "unitprice", "the unit price (shown in the property desired)"},
               {RPCResult::Type::STR, "status" , "the status of the order (\"open\", \"cancelled\", \"filled\", ...)"},
               {RPCResult::Type::STR_HEX, "canceltxid", "the hash of the transaction that cancelled the order (if cancelled)"},
               {RPCResult::Type::ARR, "matches", "a list of matched orders and executed trades",
               {
                   {RPCResult::Type::OBJ, "", "",
                   {
                       {RPCResult::Type::STR_HEX, "txid", "the hash of the transaction that was matched against"},
                       {RPCResult::Type::NUM, "block", "the index of the block that contains this transaction"},
                       {RPCResult::Type::STR, "address", "the CounosH address of the other trader"},
                       {RPCResult::Type::STR_AMOUNT, "amountsold", "the number of tokens sold in this trade"},
                       {RPCResult::Type::STR_AMOUNT, "amountreceived", "the number of tokens traded in exchange"},
                   }},
               }},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_gettrade", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
           + HelpExampleRpc("counos_gettrade", "\"921f920dd24ea072d4a550138baf16bbda096d580c80dde0f7ec6eb9023bff5c\"")
       }
    }.Check(request);

    uint256 hash = ParseHashV(request.params[0], "txid");

    UniValue txobj(UniValue::VOBJ);
    int populateResult = populateRPCTransactionObject(hash, txobj, "", true, "", pWallet.get());
    if (populateResult != 0) PopulateFailure(populateResult);

    return txobj;
}

static UniValue counos_getcurrentconsensushash(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getcurrentconsensushash",
       "\nReturns the consensus hash for all balances for the current block.\n",
       {},
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "block", "the index of the block this consensus hash applies to"},
               {RPCResult::Type::STR_HEX, "blockhash", "the hash of the corresponding block"},
               {RPCResult::Type::STR_HEX, "consensushash", "the consensus hash for the block"},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getcurrentconsensushash", "")
           + HelpExampleRpc("counos_getcurrentconsensushash", "")
       }
    }.Check(request);

    LOCK(cs_main); // TODO - will this ensure we don't take in a new block in the couple of ms it takes to calculate the consensus hash?

    int block = GetHeight();

    CBlockIndex* pblockindex = ::ChainActive()[block];
    uint256 blockHash = pblockindex->GetBlockHash();

    uint256 consensusHash = GetConsensusHash();

    UniValue response(UniValue::VOBJ);
    response.pushKV("block", block);
    response.pushKV("blockhash", blockHash.GetHex());
    response.pushKV("consensushash", consensusHash.GetHex());

    return response;
}

static UniValue counos_getmetadexhash(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getmetadexhash",
       "\nReturns a hash of the current state of the MetaDEx (default) or orderbook.\n",
       {
           {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::OMITTED, "hash orderbook (only trades selling propertyid)"},
       },
       RPCResult{
           RPCResult::Type::OBJ, "", "",
           {
               {RPCResult::Type::NUM, "block", "the index of the block this hash applies to"},
               {RPCResult::Type::STR_HEX, "blockhash", "the hash of the corresponding block"},
               {RPCResult::Type::NUM, "propertyid", "the market this hash applies to (or 0 for all markets)"},
               {RPCResult::Type::STR_HEX, "metadexhash", "the hash for the state of the MetaDEx/orderbook"},
           }
       },
       RPCExamples{
           HelpExampleCli("counos_getmetadexhash", "3")
           + HelpExampleRpc("counos_getmetadexhash", "3")
       }
    }.Check(request);

    LOCK(cs_main);

    uint32_t propertyId = 0;
    if (request.params.size() > 0) {
        propertyId = ParsePropertyId(request.params[0]);
        RequireExistingProperty(propertyId);
    }

    int block = GetHeight();
    CBlockIndex* pblockindex = ::ChainActive()[block];
    uint256 blockHash = pblockindex->GetBlockHash();

    uint256 metadexHash = GetMetaDExHash(propertyId);

    UniValue response(UniValue::VOBJ);
    response.pushKV("block", block);
    response.pushKV("blockhash", blockHash.GetHex());
    response.pushKV("propertyid", (uint64_t)propertyId);
    response.pushKV("metadexhash", metadexHash.GetHex());

    return response;
}

static UniValue counos_getbalanceshash(const JSONRPCRequest& request)
{
    RPCHelpMan{"counos_getbalanceshash",
        "counos_getbalanceshash propertyid\n",
        {
            {"propertyid", RPCArg::Type::NUM, RPCArg::Optional::NO, "the property to hash balances for"},
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::NUM, "block", "the index of the block this hash applies to"},
                {RPCResult::Type::STR_HEX, "blockhash", "the hash of the corresponding block"},
                {RPCResult::Type::NUM, "propertyid", "the property id of the hashed balances"},
                {RPCResult::Type::STR_HEX, "balanceshash", "the hash for the balances"},
            }
        },
        RPCExamples{
            HelpExampleCli("counos_getbalanceshash", "31")
            + HelpExampleRpc("counos_getbalanceshash", "31")
        }
    }.Check(request);

    LOCK(cs_main);

    uint32_t propertyId = ParsePropertyId(request.params[0]);
    RequireExistingProperty(propertyId);

    int block = GetHeight();
    CBlockIndex* pblockindex = ::ChainActive()[block];
    uint256 blockHash = pblockindex->GetBlockHash();

    uint256 balancesHash = GetBalancesHash(propertyId);

    UniValue response(UniValue::VOBJ);
    response.pushKV("block", block);
    response.pushKV("blockhash", blockHash.GetHex());
    response.pushKV("propertyid", (uint64_t)propertyId);
    response.pushKV("balanceshash", balancesHash.GetHex());

    return response;
}

static const CRPCCommand commands[] =
{ //  category                             name                            actor (function)               argNames
  //  ------------------------------------ ------------------------------- ------------------------------ ----------
    { "counos layer (data retrieval)", "counos_getinfo",                   &counos_getinfo,                    {} },
    { "counos layer (data retrieval)", "counos_getactivations",            &counos_getactivations,             {} },
    { "counos layer (data retrieval)", "counos_getallbalancesforid",       &counos_getallbalancesforid,        {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getbalance",                &counos_getbalance,                 {"address", "propertyid"} },
    { "counos layer (data retrieval)", "counos_gettransaction",            &counos_gettransaction,             {"txid"} },
    { "counos layer (data retrieval)", "counos_getproperty",               &counos_getproperty,                {"propertyid"} },
    { "counos layer (data retrieval)", "counos_listproperties",            &counos_listproperties,             {} },
    { "counos layer (data retrieval)", "counos_getcrowdsale",              &counos_getcrowdsale,               {"propertyid", "verbose"} },
    { "counos layer (data retrieval)", "counos_getgrants",                 &counos_getgrants,                  {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getactivedexsells",         &counos_getactivedexsells,          {"address"} },
    { "counos layer (data retrieval)", "counos_getactivecrowdsales",       &counos_getactivecrowdsales,        {} },
    { "counos layer (data retrieval)", "counos_getorderbook",              &counos_getorderbook,               {"propertyid", "propertyid"} },
    { "counos layer (data retrieval)", "counos_gettrade",                  &counos_gettrade,                   {"txid"} },
    { "counos layer (data retrieval)", "counos_getsto",                    &counos_getsto,                     {"txid", "recipientfilter"} },
    { "counos layer (data retrieval)", "counos_listblocktransactions",     &counos_listblocktransactions,      {"index"} },
    { "counos layer (data retrieval)", "counos_listblockstransactions",    &counos_listblockstransactions,     {"firstblock", "lastblock"} },
    { "counos layer (data retrieval)", "counos_listpendingtransactions",   &counos_listpendingtransactions,    {"address"} },
    { "counos layer (data retrieval)", "counos_getallbalancesforaddress",  &counos_getallbalancesforaddress,   {"address"} },
    { "counos layer (data retrieval)", "counos_gettradehistoryforaddress", &counos_gettradehistoryforaddress,  {"address", "count", "propertyid"} },
    { "counos layer (data retrieval)", "counos_gettradehistoryforpair",    &counos_gettradehistoryforpair,     {"propertyid", "propertyidsecond", "count"} },
    { "counos layer (data retrieval)", "counos_getcurrentconsensushash",   &counos_getcurrentconsensushash,    {} },
    { "counos layer (data retrieval)", "counos_getpayload",                &counos_getpayload,                 {"txid"} },
    { "counos layer (data retrieval)", "counos_getseedblocks",             &counos_getseedblocks,              {"startblock", "endblock"} },
    { "counos layer (data retrieval)", "counos_getmetadexhash",            &counos_getmetadexhash,             {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getfeecache",               &counos_getfeecache,                {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getfeetrigger",             &counos_getfeetrigger,              {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getfeedistribution",        &counos_getfeedistribution,         {"distributionid"} },
    { "counos layer (data retrieval)", "counos_getfeedistributions",       &counos_getfeedistributions,        {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getbalanceshash",           &counos_getbalanceshash,            {"propertyid"} },
    { "counos layer (data retrieval)", "counos_getnonfungibletokens",      &counos_getnonfungibletokens,       {"address", "propertyid"} },
    { "counos layer (data retrieval)", "counos_getnonfungibletokendata",   &counos_getnonfungibletokendata,    {"propertyid", "tokenidstart", "tokenidend"} },
    { "counos layer (data retrieval)", "counos_getnonfungibletokenranges", &counos_getnonfungibletokenranges,  {"propertyid"} },

#ifdef ENABLE_WALLET
    { "counos layer (data retrieval)", "counos_listtransactions",          &counos_listtransactions,           {"address", "count", "skip", "startblock", "endblock"} },
    { "counos layer (data retrieval)", "counos_getfeeshare",               &counos_getfeeshare,                {"address", "ecosystem"} },
    { "counos layer (configuration)",  "counos_setautocommit",             &counos_setautocommit,              {"flag"}  },
    { "counos layer (data retrieval)", "counos_getwalletbalances",         &counos_getwalletbalances,          {"includewatchonly"} },
    { "counos layer (data retrieval)", "counos_getwalletaddressbalances",  &counos_getwalletaddressbalances,   {"includewatchonly"} },
#endif
    { "hidden",                      "mscrpc",                         &mscrpc,                          {"extra", "extra2", "extra3"}  },

    /* deprecated: */
    { "hidden",                      "getinfo_MP",                     &counos_getinfo,                    {}  },
    { "hidden",                      "getbalance_MP",                  &counos_getbalance,                 {"address", "propertyid"} },
    { "hidden",                      "getallbalancesforaddress_MP",    &counos_getallbalancesforaddress,   {"address"} },
    { "hidden",                      "getallbalancesforid_MP",         &counos_getallbalancesforid,        {"propertyid"} },
    { "hidden",                      "getproperty_MP",                 &counos_getproperty,                {"propertyid"} },
    { "hidden",                      "listproperties_MP",              &counos_listproperties,             {} },
    { "hidden",                      "getcrowdsale_MP",                &counos_getcrowdsale,               {"propertyid", "verbose"} },
    { "hidden",                      "getgrants_MP",                   &counos_getgrants,                  {"propertyid"} },
    { "hidden",                      "getactivedexsells_MP",           &counos_getactivedexsells,          {"address"} },
    { "hidden",                      "getactivecrowdsales_MP",         &counos_getactivecrowdsales,        {} },
    { "hidden",                      "getsto_MP",                      &counos_getsto,                     {"txid", "recipientfilter"} },
    { "hidden",                      "getorderbook_MP",                &counos_getorderbook,               {"propertyid", "propertyiddesired"} },
    { "hidden",                      "gettrade_MP",                    &counos_gettrade,                   {"txid"} },
    { "hidden",                      "gettransaction_MP",              &counos_gettransaction,             {"txid"} },
    { "hidden",                      "listblocktransactions_MP",       &counos_listblocktransactions,      {"index"} },
#ifdef ENABLE_WALLET
    { "hidden",                      "listtransactions_MP",            &counos_listtransactions,           {"address", "count", "skip", "startblock", "endblock"} },
#endif
};

void RegisterCounosDataRetrievalRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
