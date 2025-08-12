#ifndef _PLAYERBOT_RAIDTEMPESTKEEPSTRATEGY_H
#define _PLAYERBOT_RAIDTEMPESTKEEPSTRATEGY_H

#include "Strategy.h"

class TempestKeepStrategy : public Strategy
{
public:
    TempestKeepStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "tempest keep"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif