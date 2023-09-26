#ifndef COUNOSH_COUNOSCORE_ENCODING_H
#define COUNOSH_COUNOSCORE_ENCODING_H

class CPubKey;
class CTxOut;

#include <amount.h>
#include <script/script.h>

#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

/** Embeds a payload in obfuscated multisig outputs, and adds an Exodus marker output. */
bool CounosCore_Encode_ClassB(const std::string& senderAddress, const CPubKey& redeemingPubKey, const std::vector<unsigned char>& vchPayload,
                            std::vector<std::pair<CScript, int64_t> >& vecOutputs, CAmount* outputAmount = nullptr);

/** Embeds a payload in an OP_RETURN output, prefixed with a transaction marker. */
bool CounosCore_Encode_ClassC(const std::vector<unsigned char>& vecPayload, std::vector<std::pair<CScript, int64_t> >& vecOutputs);


#endif // COUNOSH_COUNOSCORE_ENCODING_H
