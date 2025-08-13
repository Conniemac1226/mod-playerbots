#pragma once
#include "NamedObjectContext.h"
#include "MechanarTriggers.h"

class MechanarTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MechanarTriggerContext()
    {
        // Mechano Lord Capacitus
        creators["capacitus engaged"] = &MechanarTriggerContext::capacitus_engaged;
        creators["reflective shield active"] = &MechanarTriggerContext::reflective_shield_active;
        creators["polarity shift active"] = &MechanarTriggerContext::polarity_shift_active;
        creators["nether charge active"] = &MechanarTriggerContext::nether_charge_active;
        
        // Nethermancer Sepethrea
        creators["sepethrea engaged"] = &MechanarTriggerContext::sepethrea_engaged;
        creators["raging flames active"] = &MechanarTriggerContext::raging_flames_active;
        creators["should target raging flames"] = &MechanarTriggerContext::should_target_raging_flames;
        creators["dragons breath danger"] = &MechanarTriggerContext::dragons_breath_danger;
        creators["inferno danger"] = &MechanarTriggerContext::inferno_danger;
        
        // Pathaleon the Calculator
        creators["pathaleon engaged"] = &MechanarTriggerContext::pathaleon_engaged;
        creators["domination active"] = &MechanarTriggerContext::domination_active;
        creators["nether wraith active"] = &MechanarTriggerContext::nether_wraith_active;
        creators["arcane torrent danger"] = &MechanarTriggerContext::arcane_torrent_danger;
        creators["pathaleon enraged"] = &MechanarTriggerContext::pathaleon_enraged;
        creators["arcane explosion danger"] = &MechanarTriggerContext::arcane_explosion_danger;
        creators["mana tap active"] = &MechanarTriggerContext::mana_tap_active;
    }

private:
    // Mechano Lord Capacitus
    static Trigger* capacitus_engaged(PlayerbotAI* botAI) { return new CapacitusEngagedTrigger(botAI); }
    static Trigger* reflective_shield_active(PlayerbotAI* botAI) { return new ReflectiveShieldActiveTrigger(botAI); }
    static Trigger* polarity_shift_active(PlayerbotAI* botAI) { return new PolarityShiftActiveTrigger(botAI); }
    static Trigger* nether_charge_active(PlayerbotAI* botAI) { return new NetherChargeActiveTrigger(botAI); }
    
    // Nethermancer Sepethrea
    static Trigger* sepethrea_engaged(PlayerbotAI* botAI) { return new SepethreaEngagedTrigger(botAI); }
    static Trigger* raging_flames_active(PlayerbotAI* botAI) { return new RagingFlamesActiveTrigger(botAI); }
    static Trigger* should_target_raging_flames(PlayerbotAI* botAI) { return new RagingFlamesTargetTrigger(botAI); }
    static Trigger* dragons_breath_danger(PlayerbotAI* botAI) { return new DragonsBreathDangerTrigger(botAI); }
    static Trigger* inferno_danger(PlayerbotAI* botAI) { return new InfernoDangerTrigger(botAI); }
    
    // Pathaleon the Calculator
    static Trigger* pathaleon_engaged(PlayerbotAI* botAI) { return new PathaleonEngagedTrigger(botAI); }
    static Trigger* domination_active(PlayerbotAI* botAI) { return new DominationActiveTrigger(botAI); }
    static Trigger* nether_wraith_active(PlayerbotAI* botAI) { return new NetherWraithActiveTrigger(botAI); }
    static Trigger* arcane_torrent_danger(PlayerbotAI* botAI) { return new ArcaneTorrentDangerTrigger(botAI); }
    static Trigger* pathaleon_enraged(PlayerbotAI* botAI) { return new PathaleonEnragedTrigger(botAI); }
    static Trigger* arcane_explosion_danger(PlayerbotAI* botAI) { return new ArcaneExplosionDangerTrigger(botAI); }
    static Trigger* mana_tap_active(PlayerbotAI* botAI) { return new ManaTapActiveTrigger(botAI); }
};