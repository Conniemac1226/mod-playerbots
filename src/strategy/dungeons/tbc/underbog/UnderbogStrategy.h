#ifndef _PLAYERBOT_UNDERBOGSTRATEGY_H
#define _PLAYERBOT_UNDERBOGSTRATEGY_H

#include "Strategy.h"

class UnderbogStrategy : public Strategy
{
public:
    UnderbogStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "underbog"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif