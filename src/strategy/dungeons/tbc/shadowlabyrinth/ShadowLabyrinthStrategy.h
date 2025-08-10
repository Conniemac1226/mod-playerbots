#ifndef _PLAYERBOT_SHADOWLABYRINTHSTRATEGY_H
#define _PLAYERBOT_SHADOWLABYRINTHSTRATEGY_H

#include "Strategy.h"

class ShadowLabyrinthStrategy : public Strategy
{
public:
    ShadowLabyrinthStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "shadow labyrinth"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif