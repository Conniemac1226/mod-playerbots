#pragma once
#include "Strategy.h"
#include "Multiplier.h"

class PortalAddMultiplier : public Multiplier
{
public:
    PortalAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "portal add") {}
    float GetValue(Action* action) override;
};

class BlackMorassStrategy : public Strategy
{
public:
    BlackMorassStrategy(PlayerbotAI* botAI);
    std::string const getName() override { return "blackmorass"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};