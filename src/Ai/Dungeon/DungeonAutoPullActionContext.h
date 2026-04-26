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
    }

private:
    static Action* dungeon_auto_pull(PlayerbotAI* ai) { return new DungeonAutoPullAction(ai); }
};

#endif
