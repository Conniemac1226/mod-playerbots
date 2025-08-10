#ifndef _PLAYERBOT_BOTANICASTRATEGY_H
#define _PLAYERBOT_BOTANICASTRATEGY_H

#include "Strategy.h"

class BotanicaStrategy : public Strategy
{
public:
    BotanicaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "botanica"; }
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif