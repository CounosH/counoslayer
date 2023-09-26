#include <counoscore/seedblocks.h>

#include <counoscore/log.h>

#include <chainparams.h>
#include <util/time.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <stdint.h>
#include <set>
#include <string>
#include <vector>

const int MAX_SEED_BLOCK = 102121;

static std::set<int> GetBlockList()
{
    int blocks[] = {
//                    102121, 64720

//                    489970, 489971, 489972, 489974, 489975, 489976, 489977, 489978, 489980, 489981, 489982, 489983, 489984, 489985, 489986,
//                    489987, 489988, 489990, 489991, 489992, 489993, 489994, 489995, 489996, 489997, 489998, 489999, 490000
                    };

    PrintToLog("Seed block filter active - %d blocks will be parsed during initial scan.\n", sizeof(blocks)/sizeof(blocks[0]));

    return std::set<int>(blocks, blocks + sizeof(blocks)/sizeof(blocks[0]));
}

bool SkipBlock(int nBlock)
{
    static std::set<int> blockList = GetBlockList();

    // Scan all non mainnet blocks:
    if (Params().NetworkIDString() != "main") {
        return false;
    }
    // Scan all blocks, which are not in the list:
    if (nBlock > MAX_SEED_BLOCK) {
        return false;
    }
    // Otherwise check, if the block is in the list:
    return (blockList.find(nBlock) == blockList.end());
}
