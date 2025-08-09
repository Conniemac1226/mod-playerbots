#pragma once

#include "Action.h"
#include "NamedObjectContext.h"
#include "strategy/dungeons/tbc/steamvault/SteamvaultActions.h"

class SteamvaultActionContext : public NamedObjectContext<Action>
{
public:
    SteamvaultActionContext()
    {
        // Hydromancer Thespia
        creators["avoid lightning cloud"] = &SteamvaultActionContext::avoid_lightning_cloud;
        creators["dispel lung burst"] = &SteamvaultActionContext::dispel_lung_burst;
        creators["attack water elemental"] = &SteamvaultActionContext::attack_water_elemental;

        // Mekgineer Steamrigger
        creators["dispel shrink ray"] = &SteamvaultActionContext::dispel_shrink_ray;
        creators["avoid saw blade"] = &SteamvaultActionContext::avoid_saw_blade;
        creators["remove electrified net"] = &SteamvaultActionContext::remove_electrified_net;
        creators["attack steamrigger mechanic"] = &SteamvaultActionContext::attack_steamrigger_mechanic;

        // Warlord Kalithresh
        creators["stop casting spell reflection"] = &SteamvaultActionContext::stop_casting_spell_reflection;
        creators["heal impale target"] = &SteamvaultActionContext::heal_impale_target;
        creators["attack naga distiller"] = &SteamvaultActionContext::attack_naga_distiller;
        creators["interrupt distiller channel"] = &SteamvaultActionContext::interrupt_distiller_channel;
    }

private:
    // Hydromancer Thespia
    static Action* avoid_lightning_cloud(PlayerbotAI* ai) { return new AvoidLightningCloudAction(ai); }
    static Action* dispel_lung_burst(PlayerbotAI* ai) { return new DispelLungBurstAction(ai); }
    static Action* attack_water_elemental(PlayerbotAI* ai) { return new AttackWaterElementalAction(ai); }

    // Mekgineer Steamrigger
    static Action* dispel_shrink_ray(PlayerbotAI* ai) { return new DispelShrinkRayAction(ai); }
    static Action* avoid_saw_blade(PlayerbotAI* ai) { return new AvoidSawBladeAction(ai); }
    static Action* remove_electrified_net(PlayerbotAI* ai) { return new RemoveElectrifiedNetAction(ai); }
    static Action* attack_steamrigger_mechanic(PlayerbotAI* ai) { return new AttackSteamriggerMechanicAction(ai); }

    // Warlord Kalithresh
    static Action* stop_casting_spell_reflection(PlayerbotAI* ai) { return new StopCastingSpellReflectionAction(ai); }
    static Action* heal_impale_target(PlayerbotAI* ai) { return new HealImpaleTargetAction(ai); }
    static Action* attack_naga_distiller(PlayerbotAI* ai) { return new AttackNagaDistillerAction(ai); }
    static Action* interrupt_distiller_channel(PlayerbotAI* ai) { return new InterruptDistillerChannelAction(ai); }
};