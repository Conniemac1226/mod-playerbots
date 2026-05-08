#pragma once

#include "Strategy.h"

class RaidOsStrategy : public Strategy
{
public:
    RaidOsStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    std::string const getName() override { return "obsidian sanctum"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};
