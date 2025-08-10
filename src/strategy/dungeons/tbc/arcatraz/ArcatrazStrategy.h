#ifndef _PLAYERBOT_ARCATRAZSTRATEGY_H
#define _PLAYERBOT_ARCATRAZSTRATEGY_H

#include "Strategy.h"

class ArcatrazStrategy : public Strategy
{
public:
    ArcatrazStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "arcatraz"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif