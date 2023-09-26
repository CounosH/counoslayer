#ifndef COUNOSH_COUNOSCORE_RPCTXOBJECT_H
#define COUNOSH_COUNOSCORE_RPCTXOBJECT_H

#include <univalue.h>

#include <string>

class uint256;
class CMPTransaction;
class CTransaction;

namespace interfaces {
class Wallet;
} // namespace interfaces

int populateRPCTransactionObject(const uint256& txid, UniValue& txobj, std::string filterAddress = "", bool extendedDetails = false, std::string extendedDetailsFilter = "", interfaces::Wallet* iWallet = nullptr);
int populateRPCTransactionObject(const CTransaction& tx, const uint256& blockHash, UniValue& txobj, std::string filterAddress = "", bool extendedDetails = false, std::string extendedDetailsFilter = "", int blockHeight = 0, interfaces::Wallet* iWallet = nullptr);

void populateRPCTypeInfo(CMPTransaction& mp_obj, UniValue& txobj, uint32_t txType, bool extendedDetails, std::string extendedDetailsFilter, int confirmations, interfaces::Wallet* iWallet = nullptr);

void populateRPCTypeSimpleSend(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeSendToOwners(CMPTransaction& counosObj, UniValue& txobj, bool extendedDetails, std::string extendedDetailsFilter, interfaces::Wallet* iWallet = nullptr);
void populateRPCTypeSendAll(CMPTransaction& counosObj, UniValue& txobj, int confirmations);
void populateRPCTypeTradeOffer(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeMetaDExTrade(CMPTransaction& counosObj, UniValue& txobj, bool extendedDetails);
void populateRPCTypeMetaDExCancelPrice(CMPTransaction& counosObj, UniValue& txobj, bool extendedDetails);
void populateRPCTypeMetaDExCancelPair(CMPTransaction& counosObj, UniValue& txobj, bool extendedDetails);
void populateRPCTypeMetaDExCancelEcosystem(CMPTransaction& counosObj, UniValue& txobj, bool extendedDetails);
void populateRPCTypeAcceptOffer(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeCreatePropertyFixed(CMPTransaction& counosObj, UniValue& txobj, int confirmations);
void populateRPCTypeCreatePropertyVariable(CMPTransaction& counosObj, UniValue& txobj, int confirmations);
void populateRPCTypeCreatePropertyManual(CMPTransaction& counosObj, UniValue& txobj, int confirmations);
void populateRPCTypeCloseCrowdsale(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeGrant(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeRevoke(CMPTransaction& counosOobj, UniValue& txobj);
void populateRPCTypeSendNonFungible(CMPTransaction& counosOobj, UniValue& txobj);
void populateRPCTypeSetNonFungibleData(CMPTransaction& counosOobj, UniValue& txobj);
void populateRPCTypeChangeIssuer(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeActivation(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeEnableFreezing(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeDisableFreezing(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeFreezeTokens(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeUnfreezeTokens(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeAddDelegate(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeRemoveDelegate(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCTypeAnyData(CMPTransaction& counosObj, UniValue& txobj);

void populateRPCExtendedTypeSendToOwners(const uint256 txid, std::string extendedDetailsFilter, UniValue& txobj, uint16_t version, interfaces::Wallet* iWallet = nullptr);
void populateRPCExtendedTypeGrantNonFungible(CMPTransaction& counosObj, UniValue& txobj);
void populateRPCExtendedTypeMetaDExTrade(const uint256& txid, uint32_t propertyIdForSale, int64_t amountForSale, UniValue& txobj);
void populateRPCExtendedTypeMetaDExCancel(const uint256& txid, UniValue& txobj);

int populateRPCDExPurchases(const CTransaction& wtx, UniValue& purchases, std::string filterAddress, interfaces::Wallet* iWallet = nullptr);
int populateRPCSendAllSubSends(const uint256& txid, UniValue& subSends);

bool showRefForTx(uint32_t txType);

#endif // COUNOSH_COUNOSCORE_RPCTXOBJECT_H
