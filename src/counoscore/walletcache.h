#ifndef COUNOSH_COUNOSCORE_WALLETCACHE_H
#define COUNOSH_COUNOSCORE_WALLETCACHE_H

class uint256;

#include <vector>

namespace mastercore
{
/** Updates the cache and returns whether any wallet addresses were changed */
int WalletCacheUpdate();
}

#endif // COUNOSH_COUNOSCORE_WALLETCACHE_H
