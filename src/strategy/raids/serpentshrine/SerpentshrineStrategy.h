#ifndef _PLAYERBOT_SERPENTSHRINESTRATEGY_H
#define _PLAYERBOT_SERPENTSHRINESTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"

class SerpentshrineStrategy : public Strategy
{
public:
    SerpentshrineStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "serpentshrine"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif