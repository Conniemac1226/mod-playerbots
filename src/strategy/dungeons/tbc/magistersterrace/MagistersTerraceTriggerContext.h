#pragma once

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "strategy/dungeons/tbc/magistersterrace/MagistersTerraceTriggers.h"

class MagistersTerraceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MagistersTerraceTriggerContext()
    {
        // Kael'thas
        creators["kaelthas casting pyroblast"] = &MagistersTerraceTriggerContext::kaelthas_casting_pyroblast;
        creators["kaelthas casting gravity lapse"] = &MagistersTerraceTriggerContext::kaelthas_casting_gravity_lapse;
        creators["kaelthas arcane sphere nearby"] = &MagistersTerraceTriggerContext::kaelthas_arcane_sphere_nearby;

        // Vexallus
        creators["vexallus pure energy spawned"] = &MagistersTerraceTriggerContext::vexallus_pure_energy_spawned;

        // Selin Fireheart
        creators["selin fireheart fel explosion"] = &MagistersTerraceTriggerContext::selin_fireheart_fel_explosion;
        creators["selin fireheart channeling"] = &MagistersTerraceTriggerContext::selin_fireheart_channeling;
        creators["fel crystal nearby"] = &MagistersTerraceTriggerContext::fel_crystal_nearby;

        // Delrissa
        creators["delrissa add active"] = &MagistersTerraceTriggerContext::delrissa_add_active;
    }

private:
    // Kael'thas
    static Trigger* kaelthas_casting_pyroblast(PlayerbotAI* ai) { return new KaelthasCastingPyroblastTrigger(ai); }
    static Trigger* kaelthas_casting_gravity_lapse(PlayerbotAI* ai) { return new KaelthasCastingGravityLapseTrigger(ai); }
    static Trigger* kaelthas_arcane_sphere_nearby(PlayerbotAI* ai) { return new KaelthasArcaneSphereNearbyTrigger(ai); }

    // Vexallus
    static Trigger* vexallus_pure_energy_spawned(PlayerbotAI* ai) { return new VexallusPureEnergySpawnedTrigger(ai); }

    // Selin Fireheart
    static Trigger* selin_fireheart_fel_explosion(PlayerbotAI* ai) { return new SelinFireheartFelExplosionTrigger(ai); }
    static Trigger* selin_fireheart_channeling(PlayerbotAI* ai) { return new SelinFireheartChannelingTrigger(ai); }
    static Trigger* fel_crystal_nearby(PlayerbotAI* ai) { return new FelCrystalNearbyTrigger(ai); }

    // Delrissa
    static Trigger* delrissa_add_active(PlayerbotAI* ai) { return new DelrissaAddActiveTrigger(ai); }
};
