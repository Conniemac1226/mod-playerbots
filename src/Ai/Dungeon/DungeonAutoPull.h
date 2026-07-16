#ifndef _PLAYERBOT_DUNGEON_AUTO_PULL_H
#define _PLAYERBOT_DUNGEON_AUTO_PULL_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "Trigger.h"

namespace DungeonAutoPull
{
    void AddDefaultPullTrigger(std::vector<TriggerNode*>& triggers);
    void AddDefaultPullTrigger(std::vector<TriggerNode*>& triggers, float relevance);
}

class DungeonAutoPullReadyTrigger : public Trigger
{
public:
    DungeonAutoPullReadyTrigger(PlayerbotAI* ai) : Trigger(ai, "dungeon auto pull ready", 3) {}
    bool IsActive() override;
};

class DungeonAutoPullAction : public AttackAction
{
public:
    DungeonAutoPullAction(PlayerbotAI* ai) : AttackAction(ai, "dungeon auto pull") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DungeonHealerRegroupTrigger : public Trigger
{
public:
    DungeonHealerRegroupTrigger(PlayerbotAI* ai) : Trigger(ai, "dungeon healer regroup", 4) {}
    bool IsActive() override;
};

class DungeonHealerRegroupAction : public MovementAction
{
public:
    DungeonHealerRegroupAction(PlayerbotAI* ai) : MovementAction(ai, "dungeon healer regroup") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
