#pragma once
#include "Strategy.h"

class BlackMorassStrategy : public Strategy
{
public:
    BlackMorassStrategy(PlayerbotAI* botAI);
    std::string const getName() override { return "blackmorass"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};