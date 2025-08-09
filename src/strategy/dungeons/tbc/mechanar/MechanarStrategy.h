#pragma once
#include "Strategy.h"

class MechanarStrategy : public Strategy
{
public:
    MechanarStrategy(PlayerbotAI* botAI);
    std::string const getName() override { return "mechanar"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};