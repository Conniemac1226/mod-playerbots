#ifndef _PLAYERBOT_TBCDUNGEONSHSTRATEGY_H
#define _PLAYERBOT_TBCDUNGEONSHSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class TbcDungeonSHStrategy : public Strategy
{
public:
    TbcDungeonSHStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "sethekk halls"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif