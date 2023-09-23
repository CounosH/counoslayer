// Copyright (c) 2009-2018 The CounosH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COUNOSH_RPC_REGISTER_H
#define COUNOSH_RPC_REGISTER_H

/** These are in one header file to avoid creating tons of single-function
 * headers for everything under src/rpc/ */
class CRPCTable;

/** Register block chain RPC commands */
void RegisterBlockchainRPCCommands(CRPCTable &tableRPC);
/** Register P2P networking RPC commands */
void RegisterNetRPCCommands(CRPCTable &tableRPC);
/** Register miscellaneous RPC commands */
void RegisterMiscRPCCommands(CRPCTable &tableRPC);
/** Register mining RPC commands */
void RegisterMiningRPCCommands(CRPCTable &tableRPC);
/** Register raw transaction RPC commands */
void RegisterRawTransactionRPCCommands(CRPCTable &tableRPC);

/** Register Counos data retrieval RPC commands */
void RegisterCounosDataRetrievalRPCCommands(CRPCTable &tableRPC);
#ifdef ENABLE_WALLET
/** Register Counos transaction creation RPC commands */
void RegisterCounosTransactionCreationRPCCommands(CRPCTable &tableRPC);
#endif
/** Register Counos payload creation RPC commands */
void RegisterCounosPayloadCreationRPCCommands(CRPCTable &tableRPC);
/** Register Counos raw transaction RPC commands */
void RegisterCounosRawTransactionRPCCommands(CRPCTable &tableRPC);

static inline void RegisterAllCoreRPCCommands(CRPCTable &t)
{
    RegisterBlockchainRPCCommands(t);
    RegisterNetRPCCommands(t);
    RegisterMiscRPCCommands(t);
    RegisterMiningRPCCommands(t);
    RegisterRawTransactionRPCCommands(t);

    /* Counos Core RPCs: */
    RegisterCounosDataRetrievalRPCCommands(t);
#ifdef ENABLE_WALLET
    RegisterCounosTransactionCreationRPCCommands(t);
#endif
    RegisterCounosPayloadCreationRPCCommands(t);
    RegisterCounosRawTransactionRPCCommands(t);
}

#endif // COUNOSH_RPC_REGISTER_H
