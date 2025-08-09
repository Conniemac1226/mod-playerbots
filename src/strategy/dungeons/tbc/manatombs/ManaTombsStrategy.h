#ifndef _PLAYERBOT_MANATOMBSSTRATEGY_H
#define _PLAYERBOT_MANATOMBSSTRATEGY_H

#include "Strategy.h"

class ManaTombsStrategy : public Strategy
{
public:
    ManaTombsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "mana tombs"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif