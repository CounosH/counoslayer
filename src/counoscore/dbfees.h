#ifndef COUNOSH_COUNOSCORE_DBFEES_H
#define COUNOSH_COUNOSCORE_DBFEES_H

#include <counoscore/dbbase.h>
#include <counoscore/log.h>

#include <fs.h>
#include <stdint.h>
#include <set>
#include <string>
#include <utility>

typedef std::pair<int, int64_t> feeCacheItem;
typedef std::pair<std::string, int64_t> feeHistoryItem;

/** LevelDB based storage for the MetaDEx fee cache.
 */
class CCounosFeeCache : public CDBBase
{
public:
    CCounosFeeCache(const fs::path& path, bool fWipe);
    virtual ~CCounosFeeCache();

    /** Show Fee Cache DB statistics */
    void printStats();
    /** Show Fee Cache DB records */
    void printAll();

    /** Sets the distribution thresholds to total tokens for a property / COUNOS_FEE_THRESHOLD */
    void UpdateDistributionThresholds(uint32_t propertyId);
    /** Returns the distribution threshold for a property */
    int64_t GetDistributionThreshold(const uint32_t &propertyId);
    /** Return a set containing fee cache history items */
    std::set<feeCacheItem> GetCacheHistory(const uint32_t &propertyId);
    /** Gets the current amount of the fee cache for a property */
    int64_t GetCachedAmount(const uint32_t &propertyId);
    /** Prunes entries over 50 blocks old from the entry for a property */
    void PruneCache(const uint32_t &propertyId, int block);
    /** Rolls back the cache to an earlier state (eg in event of a reorg) - block is *inclusive* (ie entries=block will get deleted) */
    void RollBackCache(int block);
    /** Zeros a property in the fee cache */
    void ClearCache(const uint32_t &propertyId, int block);
    /** Adds a fee to the cache (eg on a completed trade) */
    void AddFee(const uint32_t &propertyId, int block, const int64_t &amount);
    /** Evaluates fee caches for all properties against threshold and executes distribution if threshold met */
    void EvalCache(const uint32_t &propertyId, int block);
    /** Performs distribution of fees */
    void DistributeCache(const uint32_t &propertyId, int block);
};

/** LevelDB based storage for the MetaDEx fee distributions.
 */
class CCounosFeeHistory : public CDBBase
{
public:
    CCounosFeeHistory(const fs::path& path, bool fWipe);
    virtual ~CCounosFeeHistory();

    /** Show Fee History DB statistics */
    void printStats();
    /** Show Fee History DB records */
    void printAll();

    /** Roll back history in event of reorg */
    void RollBackHistory(int block);
    /** Count Fee History DB records */
    int CountRecords();
    /** Record a fee distribution */
    void RecordFeeDistribution(const uint32_t &propertyId, int block, int64_t total, std::set<feeHistoryItem> feeRecipients);
    /** Retrieve the recipients for a fee distribution */
    std::set<feeHistoryItem> GetFeeDistribution(int id);
    /** Retrieve fee distributions for a property */
    std::set<int> GetDistributionsForProperty(const uint32_t &propertyId);
    /** Populate data about a fee distribution */
    bool GetDistributionData(int id, uint32_t *propertyId, int *block, int64_t *total);
};

namespace mastercore
{
    //! LevelDB based storage for the MetaDEx fee cache
    extern CCounosFeeCache* pDbFeeCache;
    //! LevelDB based storage for the MetaDEx fee distributions
    extern CCounosFeeHistory* pDbFeeHistory;
}

#endif // COUNOSH_COUNOSCORE_DBFEES_H
