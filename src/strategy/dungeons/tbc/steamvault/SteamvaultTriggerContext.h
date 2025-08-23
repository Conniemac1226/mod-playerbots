#pragma once

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "strategy/dungeons/tbc/steamvault/SteamvaultTriggers.h"

class SteamvaultTriggerContext : public NamedObjectContext<Trigger>
{
public:
    SteamvaultTriggerContext()
    {
        // Hydromancer Thespia
        creators["thespia lightning cloud"] = &SteamvaultTriggerContext::thespia_lightning_cloud;
        creators["thespia lung burst"] = &SteamvaultTriggerContext::thespia_lung_burst;
        creators["thespia water elemental active"] = &SteamvaultTriggerContext::thespia_water_elemental_active;

        // Mekgineer Steamrigger
        creators["steamrigger shrink ray"] = &SteamvaultTriggerContext::steamrigger_shrink_ray;
        creators["steamrigger saw blade"] = &SteamvaultTriggerContext::steamrigger_saw_blade;
        creators["steamrigger electrified net"] = &SteamvaultTriggerContext::steamrigger_electrified_net;
        creators["steamrigger mechanic active"] = &SteamvaultTriggerContext::steamrigger_mechanic_active;

        // Warlord Kalithresh
        creators["kalithresh spell reflection"] = &SteamvaultTriggerContext::kalithresh_spell_reflection;
        creators["kalithresh spell reflection ended"] = &SteamvaultTriggerContext::kalithresh_spell_reflection_ended;
        creators["kalithresh impale"] = &SteamvaultTriggerContext::kalithresh_impale;
        creators["kalithresh naga distiller active"] = &SteamvaultTriggerContext::kalithresh_naga_distiller_active;
        creators["kalithresh channeling rage"] = &SteamvaultTriggerContext::kalithresh_channeling_rage;
    }

private:
    // Hydromancer Thespia
    static Trigger* thespia_lightning_cloud(PlayerbotAI* ai) { return new ThespiaLightningCloudTrigger(ai); }
    static Trigger* thespia_lung_burst(PlayerbotAI* ai) { return new ThespiaLungBurstTrigger(ai); }
    static Trigger* thespia_water_elemental_active(PlayerbotAI* ai) { return new ThespiaWaterElementalActiveTrigger(ai); }

    // Mekgineer Steamrigger
    static Trigger* steamrigger_shrink_ray(PlayerbotAI* ai) { return new SteamriggerShrinkRayTrigger(ai); }
    static Trigger* steamrigger_saw_blade(PlayerbotAI* ai) { return new SteamriggerSawBladeTrigger(ai); }
    static Trigger* steamrigger_electrified_net(PlayerbotAI* ai) { return new SteamriggerElectrifiedNetTrigger(ai); }
    static Trigger* steamrigger_mechanic_active(PlayerbotAI* ai) { return new SteamriggerMechanicActiveTrigger(ai); }

    // Warlord Kalithresh
    static Trigger* kalithresh_spell_reflection(PlayerbotAI* ai) { return new KalithreshSpellReflectionTrigger(ai); }
    static Trigger* kalithresh_spell_reflection_ended(PlayerbotAI* ai) { return new KalithreshSpellReflectionEndedTrigger(ai); }
    static Trigger* kalithresh_impale(PlayerbotAI* ai) { return new KalithreshImpaleTrigger(ai); }
    static Trigger* kalithresh_naga_distiller_active(PlayerbotAI* ai) { return new KalithreshNagaDistillerActiveTrigger(ai); }
    static Trigger* kalithresh_channeling_rage(PlayerbotAI* ai) { return new KalithreshChannelingRageTrigger(ai); }
};