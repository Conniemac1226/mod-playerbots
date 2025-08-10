#ifndef _PLAYERBOT_GRUULSTRATEGY_H
#define _PLAYERBOT_GRUULSTRATEGY_H

#include "Strategy.h"

class GruulStrategy : public Strategy
{
public:
    GruulStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "gruul"; }
    
private:
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

#endif