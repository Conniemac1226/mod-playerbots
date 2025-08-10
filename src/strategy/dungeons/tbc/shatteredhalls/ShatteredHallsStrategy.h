#ifndef _PLAYERBOT_SHATTEREDHALLSSTRATEGY_H
#define _PLAYERBOT_SHATTEREDHALLSSTRATEGY_H

#include "Strategy.h"

class ShatteredHallsStrategy : public Strategy
{
public:
    ShatteredHallsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "shattered halls"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif