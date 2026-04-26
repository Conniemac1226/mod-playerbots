#ifndef _PLAYERBOT_CLASSIC_DUNGEON_STRATEGY_H
#define _PLAYERBOT_CLASSIC_DUNGEON_STRATEGY_H

#include "Strategy.h"

class ClassicDungeonStrategy : public Strategy
{
public:
    ClassicDungeonStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "classic-dungeon"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
