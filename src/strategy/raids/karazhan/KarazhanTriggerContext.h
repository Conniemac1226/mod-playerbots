#ifndef _PLAYERBOT_KARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_KARAZHANTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "KarazhanTriggers.h"

class KarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    KarazhanTriggerContext()
    {
        // Attumen
        creators["attumen engaged"] = &KarazhanTriggerContext::attumen_engaged;
        creators["attumen mounted"] = &KarazhanTriggerContext::attumen_mounted;
        creators["attumen charge danger"] = &KarazhanTriggerContext::attumen_charge_danger;
        creators["attumen shadowcleave"] = &KarazhanTriggerContext::attumen_shadowcleave;
        
        // Moroes
        creators["moroes engaged"] = &KarazhanTriggerContext::moroes_engaged;
        creators["moroes adds"] = &KarazhanTriggerContext::moroes_adds;
        creators["moroes garrote"] = &KarazhanTriggerContext::moroes_garrote;
        creators["moroes gouge"] = &KarazhanTriggerContext::moroes_gouge;
        
        // Maiden of Virtue
        creators["maiden engaged"] = &KarazhanTriggerContext::maiden_engaged;
        creators["maiden repentance"] = &KarazhanTriggerContext::maiden_repentance;
        creators["maiden holy ground"] = &KarazhanTriggerContext::maiden_holy_ground;
        
        // Opera Event
        creators["opera engaged"] = &KarazhanTriggerContext::opera_engaged;
        creators["opera oz engaged"] = &KarazhanTriggerContext::opera_oz_engaged;
        creators["opera romulo julianne"] = &KarazhanTriggerContext::opera_romulo_julianne;
        creators["opera wolf"] = &KarazhanTriggerContext::opera_wolf;
        
        // Curator
        creators["curator engaged"] = &KarazhanTriggerContext::curator_engaged;
        creators["curator flare"] = &KarazhanTriggerContext::curator_flare;
        creators["curator evocation"] = &KarazhanTriggerContext::curator_evocation;
        
        // Shade of Aran
        creators["aran engaged"] = &KarazhanTriggerContext::aran_engaged;
        creators["aran flame wreath"] = &KarazhanTriggerContext::aran_flame_wreath;
        creators["aran blizzard"] = &KarazhanTriggerContext::aran_blizzard;
        creators["aran dragons breath"] = &KarazhanTriggerContext::aran_dragons_breath;
        
        // Terestian Illhoof
        creators["illhoof engaged"] = &KarazhanTriggerContext::illhoof_engaged;
        creators["illhoof demon chains"] = &KarazhanTriggerContext::illhoof_demon_chains;
        creators["illhoof imps"] = &KarazhanTriggerContext::illhoof_imps;
        
        // Netherspite
        creators["netherspite engaged"] = &KarazhanTriggerContext::netherspite_engaged;
        creators["netherspite beams"] = &KarazhanTriggerContext::netherspite_beams;
        creators["netherspite void zone"] = &KarazhanTriggerContext::netherspite_void_zone;
        
        // Prince Malchezaar
        creators["malchezaar engaged"] = &KarazhanTriggerContext::malchezaar_engaged;
        creators["malchezaar infernal"] = &KarazhanTriggerContext::malchezaar_infernal;
        creators["malchezaar enfeeble"] = &KarazhanTriggerContext::malchezaar_enfeeble;
        
        // Nightbane
        creators["nightbane engaged"] = &KarazhanTriggerContext::nightbane_engaged;
        creators["nightbane air phase"] = &KarazhanTriggerContext::nightbane_air_phase;
        creators["nightbane charred earth"] = &KarazhanTriggerContext::nightbane_charred_earth;
        
        // Chess Event
        creators["chess event active"] = &KarazhanTriggerContext::chess_event_active;
        
        // Utility Triggers
        creators["karazhan interrupt needed"] = &KarazhanTriggerContext::karazhan_interrupt_needed;
        creators["karazhan dispel needed"] = &KarazhanTriggerContext::karazhan_dispel_needed;
    }

private:
    // Attumen
    static Trigger* attumen_engaged(PlayerbotAI* ai) { return new AttumenEngagedTrigger(ai); }
    static Trigger* attumen_mounted(PlayerbotAI* ai) { return new AttumenMountedTrigger(ai); }
    static Trigger* attumen_charge_danger(PlayerbotAI* ai) { return new AttumenChargeDangerTrigger(ai); }
    static Trigger* attumen_shadowcleave(PlayerbotAI* ai) { return new AttumenShadowcleaveTrigger(ai); }
    
    // Moroes
    static Trigger* moroes_engaged(PlayerbotAI* ai) { return new MoroesEngagedTrigger(ai); }
    static Trigger* moroes_adds(PlayerbotAI* ai) { return new MoroesAddsTrigger(ai); }
    static Trigger* moroes_garrote(PlayerbotAI* ai) { return new MoroesGarroteTrigger(ai); }
    static Trigger* moroes_gouge(PlayerbotAI* ai) { return new MoroesGarroteTrigger(ai); } // Reuse same logic
    
    // Maiden of Virtue
    static Trigger* maiden_engaged(PlayerbotAI* ai) { return new MaidenEngagedTrigger(ai); }
    static Trigger* maiden_repentance(PlayerbotAI* ai) { return new MaidenRepentanceTrigger(ai); }
    static Trigger* maiden_holy_ground(PlayerbotAI* ai) { return new MaidenHolyGroundTrigger(ai); }
    
    // Opera Event
    static Trigger* opera_engaged(PlayerbotAI* ai) { return new OperaEngagedTrigger(ai); }
    static Trigger* opera_oz_engaged(PlayerbotAI* ai) { return new OperaOzEngagedTrigger(ai); }
    static Trigger* opera_romulo_julianne(PlayerbotAI* ai) { return new OperaRomuloJulianneTrigger(ai); }
    static Trigger* opera_wolf(PlayerbotAI* ai) { return new OperaWolfTrigger(ai); }
    
    // Curator
    static Trigger* curator_engaged(PlayerbotAI* ai) { return new CuratorEngagedTrigger(ai); }
    static Trigger* curator_flare(PlayerbotAI* ai) { return new CuratorFlareTrigger(ai); }
    static Trigger* curator_evocation(PlayerbotAI* ai) { return new CuratorEvocationTrigger(ai); }
    
    // Shade of Aran
    static Trigger* aran_engaged(PlayerbotAI* ai) { return new AranEngagedTrigger(ai); }
    static Trigger* aran_flame_wreath(PlayerbotAI* ai) { return new AranFlameWreathTrigger(ai); }
    static Trigger* aran_blizzard(PlayerbotAI* ai) { return new AranBlizzardTrigger(ai); }
    static Trigger* aran_dragons_breath(PlayerbotAI* ai) { return new AranDragonsBreathTrigger(ai); }
    
    // Terestian Illhoof
    static Trigger* illhoof_engaged(PlayerbotAI* ai) { return new IllhoofEngagedTrigger(ai); }
    static Trigger* illhoof_demon_chains(PlayerbotAI* ai) { return new IllhoofDemonChainsTrigger(ai); }
    static Trigger* illhoof_imps(PlayerbotAI* ai) { return new IllhoofImpsTrigger(ai); }
    
    // Netherspite
    static Trigger* netherspite_engaged(PlayerbotAI* ai) { return new NetherspiteEngagedTrigger(ai); }
    static Trigger* netherspite_beams(PlayerbotAI* ai) { return new NetherspiteBeamsTrigger(ai); }
    static Trigger* netherspite_void_zone(PlayerbotAI* ai) { return new NetherspiteVoidZoneTrigger(ai); }
    
    // Prince Malchezaar
    static Trigger* malchezaar_engaged(PlayerbotAI* ai) { return new MalchezaarEngagedTrigger(ai); }
    static Trigger* malchezaar_infernal(PlayerbotAI* ai) { return new MalchezaarInfernalTrigger(ai); }
    static Trigger* malchezaar_enfeeble(PlayerbotAI* ai) { return new MalchezaarEnfeebleTrigger(ai); }
    
    // Nightbane
    static Trigger* nightbane_engaged(PlayerbotAI* ai) { return new NightbaneEngagedTrigger(ai); }
    static Trigger* nightbane_air_phase(PlayerbotAI* ai) { return new NightbaneAirPhaseTrigger(ai); }
    static Trigger* nightbane_charred_earth(PlayerbotAI* ai) { return new NightbaneCharredEarthTrigger(ai); }
    
    // Chess Event
    static Trigger* chess_event_active(PlayerbotAI* ai) { return new MoroesEngagedTrigger(ai); } // Placeholder
    
    // Utility Triggers
    static Trigger* karazhan_interrupt_needed(PlayerbotAI* ai) { return new MoroesEngagedTrigger(ai); } // Placeholder
    static Trigger* karazhan_dispel_needed(PlayerbotAI* ai) { return new MoroesGarroteTrigger(ai); } // Use Garrote trigger
};

#endif