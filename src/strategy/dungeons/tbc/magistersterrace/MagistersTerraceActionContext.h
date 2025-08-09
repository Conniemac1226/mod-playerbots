#pragma once

#include "Action.h"
#include "NamedObjectContext.h"
#include "strategy/dungeons/tbc/magistersterrace/MagistersTerraceActions.h"

class MagistersTerraceActionContext : public NamedObjectContext<Action>
{
public:
    MagistersTerraceActionContext()
    {
        // Kael'thas
        creators["interrupt kaelthas pyroblast"] = &MagistersTerraceActionContext::interrupt_kaelthas_pyroblast;
        creators["avoid gravity lapse"] = &MagistersTerraceActionContext::avoid_gravity_lapse;
        creators["flee arcane sphere"] = &MagistersTerraceActionContext::flee_arcane_sphere;

        // Vexallus
        creators["attack pure energy"] = &MagistersTerraceActionContext::attack_pure_energy;

        // Selin Fireheart
        creators["avoid fel explosion"] = &MagistersTerraceActionContext::avoid_fel_explosion;
        creators["attack fel crystal"] = &MagistersTerraceActionContext::attack_fel_crystal;

        // Delrissa
        creators["attack delrissa add"] = &MagistersTerraceActionContext::attack_delrissa_add;
    }

private:
    // Kael'thas
    static Action* interrupt_kaelthas_pyroblast(PlayerbotAI* ai) { return new InterruptKaelthasPyroblastAction(ai); }
    static Action* avoid_gravity_lapse(PlayerbotAI* ai) { return new AvoidGravityLapseAction(ai); }
    static Action* flee_arcane_sphere(PlayerbotAI* ai) { return new FleeArcaneSphereAction(ai); }

    // Vexallus
    static Action* attack_pure_energy(PlayerbotAI* ai) { return new AttackPureEnergyAction(ai); }

    // Selin Fireheart
    static Action* avoid_fel_explosion(PlayerbotAI* ai) { return new AvoidFelExplosionAction(ai); }
    static Action* attack_fel_crystal(PlayerbotAI* ai) { return new AttackFelCrystalAction(ai); }

    // Delrissa
    static Action* attack_delrissa_add(PlayerbotAI* ai) { return new AttackDelrissaAddAction(ai); }
};
