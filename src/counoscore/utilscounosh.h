#ifndef COUNOSH_COUNOSCORE_UTILSCOUNOSH_H
#define COUNOSH_COUNOSCORE_UTILSCOUNOSH_H

class CBlockIndex;
class uint256;

#include <stdint.h>

namespace mastercore
{
/** Returns the current chain length. */
int GetHeight();
/** Returns the timestamp of the latest block. */
uint32_t GetLatestBlockTime();
/** Returns the CBlockIndex for a given block hash, or NULL. */
CBlockIndex* GetBlockIndex(const uint256& hash);

bool MainNet();
bool TestNet();
bool RegTest();
bool UnitTest();
bool isNonMainNet();
}

#endif // COUNOSH_COUNOSCORE_UTILSCOUNOSH_H
