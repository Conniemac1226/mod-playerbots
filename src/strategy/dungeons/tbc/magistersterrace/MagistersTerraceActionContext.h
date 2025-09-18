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
        creators["avoid flamestrike"] = &MagistersTerraceActionContext::avoid_flamestrike;

        // Vexallus
        creators["attack pure energy"] = &MagistersTerraceActionContext::attack_pure_energy;
        creators["spread out vexallus"] = &MagistersTerraceActionContext::spread_out_vexallus;

        // Selin Fireheart
        creators["avoid fel explosion"] = &MagistersTerraceActionContext::avoid_fel_explosion;
        creators["attack fel crystal"] = &MagistersTerraceActionContext::attack_fel_crystal;

        // Delrissa
        creators["attack delrissa add"] = &MagistersTerraceActionContext::attack_delrissa_add;
        creators["interrupt delrissa helper"] = &MagistersTerraceActionContext::interrupt_delrissa_helper;
        creators["delrissa dispel handling"] = &MagistersTerraceActionContext::delrissa_dispel_handling;
    }

private:
    // Kael'thas
    static Action* interrupt_kaelthas_pyroblast(PlayerbotAI* ai) { return new InterruptKaelthasPyroblastAction(ai); }
    static Action* avoid_gravity_lapse(PlayerbotAI* ai) { return new AvoidGravityLapseAction(ai); }
    static Action* flee_arcane_sphere(PlayerbotAI* ai) { return new FleeArcaneSphereAction(ai); }
    static Action* avoid_flamestrike(PlayerbotAI* ai) { return new AvoidFlamestrikeAction(ai); }

    // Vexallus
    static Action* attack_pure_energy(PlayerbotAI* ai) { return new AttackPureEnergyAction(ai); }
    static Action* spread_out_vexallus(PlayerbotAI* ai) { return new VexallusSpreadOutAction(ai); }

    // Selin Fireheart
    static Action* avoid_fel_explosion(PlayerbotAI* ai) { return new AvoidFelExplosionAction(ai); }
    static Action* attack_fel_crystal(PlayerbotAI* ai) { return new AttackFelCrystalAction(ai); }

    // Delrissa
    static Action* attack_delrissa_add(PlayerbotAI* ai) { return new AttackDelrissaAddAction(ai); }
    static Action* interrupt_delrissa_helper(PlayerbotAI* ai) { return new InterruptDelrissaHelperAction(ai); }
    static Action* delrissa_dispel_handling(PlayerbotAI* ai) { return new DelrissaDispelHandlingAction(ai); }
};
