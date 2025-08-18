#pragma once
#include "NamedObjectContext.h"
#include "MechanarActions.h"

class MechanarActionContext : public NamedObjectContext<Action>
{
public:
    MechanarActionContext()
    {
        // Mechano Lord Capacitus
        creators["handle reflective shield"] = &MechanarActionContext::handle_reflective_shield;
        creators["handle polarity shift"] = &MechanarActionContext::handle_polarity_shift;
        creators["attack nether charge"] = &MechanarActionContext::attack_nether_charge;
        creators["capacitus position"] = &MechanarActionContext::capacitus_position;
        
        // Nethermancer Sepethrea
        creators["flee raging flames"] = &MechanarActionContext::flee_raging_flames;
        creators["avoid raging flames inferno"] = &MechanarActionContext::avoid_raging_flames_inferno;
        creators["avoid raging flames fire trail"] = &MechanarActionContext::avoid_raging_flames_fire_trail;
        creators["target raging flames"] = &MechanarActionContext::target_raging_flames;
        creators["avoid dragons breath"] = &MechanarActionContext::avoid_dragons_breath;
        creators["handle arcane blast"] = &MechanarActionContext::handle_arcane_blast;
        
        // Pathaleon the Calculator
        creators["handle domination"] = &MechanarActionContext::handle_domination;
        creators["attack nether wraith"] = &MechanarActionContext::attack_nether_wraith;
        creators["avoid arcane torrent"] = &MechanarActionContext::avoid_arcane_torrent;
        creators["handle pathaleon enrage"] = &MechanarActionContext::handle_pathaleon_enrage;
        creators["avoid arcane explosion"] = &MechanarActionContext::avoid_arcane_explosion;
        creators["handle mana tap"] = &MechanarActionContext::handle_mana_tap;
    }

private:
    // Mechano Lord Capacitus
    static Action* handle_reflective_shield(PlayerbotAI* botAI) { return new CapacitusReflectiveShieldAction(botAI); }
    static Action* handle_polarity_shift(PlayerbotAI* botAI) { return new CapacitusPolarityShiftAction(botAI); }
    static Action* attack_nether_charge(PlayerbotAI* botAI) { return new CapacitusNetherChargeAction(botAI); }
    static Action* capacitus_position(PlayerbotAI* botAI) { return new CapacitusPositionAction(botAI); }
    
    // Nethermancer Sepethrea
    static Action* flee_raging_flames(PlayerbotAI* botAI) { return new SepethreaRagingFlamesAction(botAI); }
    static Action* avoid_raging_flames_inferno(PlayerbotAI* botAI) { return new SepethreaInfernoAvoidanceAction(botAI); }
    static Action* avoid_raging_flames_fire_trail(PlayerbotAI* botAI) { return new SepethreaFireTrailAvoidanceAction(botAI); }
    static Action* target_raging_flames(PlayerbotAI* botAI) { return new SepethreaTargetElementalAction(botAI); }
    static Action* avoid_dragons_breath(PlayerbotAI* botAI) { return new SepethreaDragonsBreathAction(botAI); }
    static Action* handle_arcane_blast(PlayerbotAI* botAI) { return new SepethreaArcaneBlastAction(botAI); }
    
    // Pathaleon the Calculator
    static Action* handle_domination(PlayerbotAI* botAI) { return new PathaleonDominationAction(botAI); }
    static Action* attack_nether_wraith(PlayerbotAI* botAI) { return new PathaleonNetherWraithAction(botAI); }
    static Action* avoid_arcane_torrent(PlayerbotAI* botAI) { return new PathaleonArcaneTorrentAction(botAI); }
    static Action* handle_pathaleon_enrage(PlayerbotAI* botAI) { return new PathaleonEnrageAction(botAI); }
    static Action* avoid_arcane_explosion(PlayerbotAI* botAI) { return new PathaleonArcaneExplosionAction(botAI); }
    static Action* handle_mana_tap(PlayerbotAI* botAI) { return new PathaleonManaTapAction(botAI); }
};