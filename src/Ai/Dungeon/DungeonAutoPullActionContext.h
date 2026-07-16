#ifndef _PLAYERBOT_DUNGEON_AUTO_PULL_ACTION_CONTEXT_H
#define _PLAYERBOT_DUNGEON_AUTO_PULL_ACTION_CONTEXT_H

#include "DungeonAutoPull.h"
#include "NamedObjectContext.h"

class DungeonAutoPullActionContext : public NamedObjectContext<Action>
{
public:
    DungeonAutoPullActionContext()
    {
        creators["dungeon auto pull"] = &DungeonAutoPullActionContext::dungeon_auto_pull;
        creators["dungeon healer regroup"] = &DungeonAutoPullActionContext::dungeon_healer_regroup;
    }

private:
    static Action* dungeon_auto_pull(PlayerbotAI* ai) { return new DungeonAutoPullAction(ai); }
    static Action* dungeon_healer_regroup(PlayerbotAI* ai) { return new DungeonHealerRegroupAction(ai); }
};

#endif
