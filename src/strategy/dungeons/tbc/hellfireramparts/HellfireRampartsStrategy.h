#ifndef _PLAYERBOT_HELLFIRERAMPARTSSTRATEGY_H
#define _PLAYERBOT_HELLFIRERAMPARTSSTRATEGY_H

#include "Strategy.h"

class TbcDungeonHRStrategy : public Strategy
{
public:
    TbcDungeonHRStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "tbc-hr"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif