#ifndef _PLAYERBOT_KARAZHANSTRATEGY_H
#define _PLAYERBOT_KARAZHANSTRATEGY_H

#include "Strategy.h"
#include "Multiplier.h"

class KarazhanStrategy : public Strategy
{
public:
    KarazhanStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "karazhan"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif