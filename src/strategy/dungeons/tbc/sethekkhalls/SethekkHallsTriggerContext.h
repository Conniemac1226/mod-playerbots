#ifndef _PLAYERBOT_TBCDUNGEONSHTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONSHTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "SethekkHallsTriggers.h"

class TbcDungeonSHTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        TbcDungeonSHTriggerContext()
        {
            creators["charming totem spawned"] = &TbcDungeonSHTriggerContext::charming_totem_spawned;
            creators["time lost controller casting totem"] = &TbcDungeonSHTriggerContext::controller_casting_totem;
            creators["ikiss blink cast"] = &TbcDungeonSHTriggerContext::ikiss_blink_cast;
            creators["ikiss arcane explosion cast"] = &TbcDungeonSHTriggerContext::ikiss_arcane_explosion_cast;
            creators["ikiss arcane explosion ended"] = &TbcDungeonSHTriggerContext::ikiss_arcane_explosion_ended;
            creators["sethekk spirit nearby"] = &TbcDungeonSHTriggerContext::sethekk_spirit_nearby;
            creators["brood of anzu nearby"] = &TbcDungeonSHTriggerContext::brood_of_anzu_nearby;
        }
    private:
        static Trigger* charming_totem_spawned(PlayerbotAI* ai) { return new CharmingTotemSpawnedTrigger(ai); }
        static Trigger* controller_casting_totem(PlayerbotAI* ai) { return new TimeLostControllerCastingTotemTrigger(ai); }
        static Trigger* ikiss_blink_cast(PlayerbotAI* ai) { return new IkissBlinkCastTrigger(ai); }
        static Trigger* ikiss_arcane_explosion_cast(PlayerbotAI* ai) { return new IkissArcaneExplosionCastTrigger(ai); }
        static Trigger* ikiss_arcane_explosion_ended(PlayerbotAI* ai) { return new IkissArcaneExplosionEndedTrigger(ai); }
        static Trigger* sethekk_spirit_nearby(PlayerbotAI* ai) { return new SethekkSpiritNearbyTrigger(ai); }
        static Trigger* brood_of_anzu_nearby(PlayerbotAI* ai) { return new BroodOfAnzuNearbyTrigger(ai); }
};

#endif