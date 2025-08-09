#ifndef _PLAYERBOT_SLAVEPENSSTRATEGY_H
#define _PLAYERBOT_SLAVEPENSSTRATEGY_H

#include "Strategy.h"

class SlavePensStrategy : public Strategy
{
public:
    SlavePensStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "slave pens"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif