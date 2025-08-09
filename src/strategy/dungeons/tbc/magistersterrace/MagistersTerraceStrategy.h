#pragma once

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"

class MagistersTerraceStrategy : public Strategy
{
public:
    MagistersTerraceStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "magisters terrace"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};
