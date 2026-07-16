#ifndef _PLAYERBOT_DUNGEON_AUTO_PULL_TRIGGER_CONTEXT_H
#define _PLAYERBOT_DUNGEON_AUTO_PULL_TRIGGER_CONTEXT_H

#include "DungeonAutoPull.h"
#include "NamedObjectContext.h"

class DungeonAutoPullTriggerContext : public NamedObjectContext<Trigger>
{
public:
    DungeonAutoPullTriggerContext()
    {
        creators["dungeon auto pull ready"] = &DungeonAutoPullTriggerContext::dungeon_auto_pull_ready;
        creators["dungeon healer regroup"] = &DungeonAutoPullTriggerContext::dungeon_healer_regroup;
    }

private:
    static Trigger* dungeon_auto_pull_ready(PlayerbotAI* ai) { return new DungeonAutoPullReadyTrigger(ai); }
    static Trigger* dungeon_healer_regroup(PlayerbotAI* ai) { return new DungeonHealerRegroupTrigger(ai); }
};

#endif
