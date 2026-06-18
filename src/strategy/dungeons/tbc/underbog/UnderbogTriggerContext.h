#ifndef _PLAYERBOT_UNDERBOGTRIGGERCONTEXT_H
#define _PLAYERBOT_UNDERBOGTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "UnderbogTriggers.h"

class UnderbogTriggerContext : public NamedObjectContext<Trigger>
{
public:
    UnderbogTriggerContext()
    {
        creators["mushroom nearby"] = &UnderbogTriggerContext::mushroom_nearby;
        creators["foul spores active"] = &UnderbogTriggerContext::foul_spores_active;
        creators["acid breath casting"] = &UnderbogTriggerContext::acid_breath_casting;
        creators["tail sweep danger"] = &UnderbogTriggerContext::tail_sweep_danger;
        creators["windcaller claw active"] = &UnderbogTriggerContext::windcaller_claw_active;
        creators["freezing trap incoming"] = &UnderbogTriggerContext::freezing_trap_incoming;
        creators["hunters mark active"] = &UnderbogTriggerContext::hunters_mark_active;
        creators["levitate active"] = &UnderbogTriggerContext::levitate_active;
        creators["static charge active"] = &UnderbogTriggerContext::static_charge_active;
        creators["spore strider active"] = &UnderbogTriggerContext::spore_strider_active;
        creators["chain lightning casting"] = &UnderbogTriggerContext::chain_lightning_casting;
    }

private:
    static Trigger* mushroom_nearby(PlayerbotAI* ai) { return new HungarfenMushroomNearbyTrigger(ai); }
    static Trigger* foul_spores_active(PlayerbotAI* ai) { return new HungarfenFoulSporesTrigger(ai); }
    static Trigger* acid_breath_casting(PlayerbotAI* ai) { return new GhazanAcidBreathTrigger(ai); }
    static Trigger* tail_sweep_danger(PlayerbotAI* ai) { return new GhazanTailSweepTrigger(ai); }
    static Trigger* windcaller_claw_active(PlayerbotAI* ai) { return new WindcallerClawActiveTrigger(ai); }
    static Trigger* freezing_trap_incoming(PlayerbotAI* ai) { return new MuselekFreezingTrapTrigger(ai); }
    static Trigger* hunters_mark_active(PlayerbotAI* ai) { return new MuselekHuntersMarkTrigger(ai); }
    static Trigger* levitate_active(PlayerbotAI* ai) { return new BlackStalkerLevitateTrigger(ai); }
    static Trigger* static_charge_active(PlayerbotAI* ai) { return new BlackStalkerStaticChargeTrigger(ai); }
    static Trigger* spore_strider_active(PlayerbotAI* ai) { return new SporeStriderActiveTrigger(ai); }
    static Trigger* chain_lightning_casting(PlayerbotAI* ai) { return new BlackStalkerChainLightningTrigger(ai); }
};

#endif
