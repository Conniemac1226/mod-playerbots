#ifndef _PLAYERBOT_TBCDUNGEONSACTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONSACTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "AuchenaiCryptsTriggers.h"

class TbcDungeonACTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        TbcDungeonACTriggerContext()
        {
            creators["shirrak focus fire spawned"] = &TbcDungeonACTriggerContext::shirrak_focus_fire_spawned;
            creators["shirrak focus fire ended"] = &TbcDungeonACTriggerContext::shirrak_focus_fire_ended;
            creators["shirrak attract magic"] = &TbcDungeonACTriggerContext::shirrak_attract_magic;
        }
    private:
        static Trigger* shirrak_focus_fire_spawned(PlayerbotAI* ai) { return new ShirrakFocusFireSpawnedTrigger(ai); }
        static Trigger* shirrak_focus_fire_ended(PlayerbotAI* ai) { return new ShirrakFocusFireEndedTrigger(ai); }
        static Trigger* shirrak_attract_magic(PlayerbotAI* ai) { return new ShirrakAttractMagicTrigger(ai); }
};

#endif