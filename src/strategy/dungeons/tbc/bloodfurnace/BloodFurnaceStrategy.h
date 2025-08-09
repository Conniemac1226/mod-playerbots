#ifndef _PLAYERBOT_BLOODFURNACESTRATEGY_H
#define _PLAYERBOT_BLOODFURNACESTRATEGY_H

#include "Strategy.h"
#include "PlayerbotAI.h"

class BloodFurnaceStrategy : public Strategy
{
public:
    BloodFurnaceStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "blood furnace"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }
};

#endif