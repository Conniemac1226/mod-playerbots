#ifndef _PLAYERBOT_KARAZHANACTIONCONTEXT_H
#define _PLAYERBOT_KARAZHANACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "KarazhanActions.h"

class KarazhanActionContext : public NamedObjectContext<Action>
{
public:
    KarazhanActionContext()
    {
        // Attumen
        creators["attumen avoid charge"] = &KarazhanActionContext::attumen_avoid_charge;
        creators["attumen position"] = &KarazhanActionContext::attumen_position;
        
        // Moroes
        creators["moroes focus adds"] = &KarazhanActionContext::moroes_focus_adds;
        creators["moroes position"] = &KarazhanActionContext::moroes_position;
        creators["moroes crowd control"] = &KarazhanActionContext::moroes_crowd_control;
        creators["moroes tank swap"] = &KarazhanActionContext::moroes_tank_swap;
        
        // Maiden of Virtue
        creators["maiden repentance"] = &KarazhanActionContext::maiden_repentance;
        creators["maiden holy ground"] = &KarazhanActionContext::maiden_holy_ground;
        
        // Opera Event
        creators["opera position"] = &KarazhanActionContext::opera_position;
        creators["opera focus target"] = &KarazhanActionContext::opera_focus_target;
        
        // Curator
        creators["curator flare"] = &KarazhanActionContext::curator_flare;
        creators["curator evocation"] = &KarazhanActionContext::curator_evocation;
        
        // Shade of Aran
        creators["aran flame wreath"] = &KarazhanActionContext::aran_flame_wreath;
        creators["aran blizzard"] = &KarazhanActionContext::aran_blizzard;
        creators["aran dragons breath"] = &KarazhanActionContext::aran_dragons_breath;
        
        // Terestian Illhoof
        creators["illhoof demon chains"] = &KarazhanActionContext::illhoof_demon_chains;
        creators["illhoof imps"] = &KarazhanActionContext::illhoof_imps;
        
        // Netherspite
        creators["netherspite beam"] = &KarazhanActionContext::netherspite_beam;
        creators["netherspite void zone"] = &KarazhanActionContext::netherspite_void_zone;
        
        // Prince Malchezaar
        creators["malchezaar infernal"] = &KarazhanActionContext::malchezaar_infernal;
        creators["malchezaar enfeeble"] = &KarazhanActionContext::malchezaar_enfeeble;
        
        // Nightbane
        creators["nightbane position"] = &KarazhanActionContext::nightbane_position;
        creators["nightbane charred earth"] = &KarazhanActionContext::nightbane_charred_earth;
        creators["nightbane air phase"] = &KarazhanActionContext::nightbane_air_phase;
        
        // Chess Event
        creators["chess event move"] = &KarazhanActionContext::chess_event_move;
        creators["chess event ability"] = &KarazhanActionContext::chess_event_ability;
        
        // Utility Actions
        creators["karazhan interrupt rotation"] = &KarazhanActionContext::karazhan_interrupt_rotation;
        creators["karazhan dispel"] = &KarazhanActionContext::karazhan_dispel;
    }

private:
    // Attumen
    static Action* attumen_avoid_charge(PlayerbotAI* ai) { return new AttumenAvoidChargeAction(ai); }
    static Action* attumen_position(PlayerbotAI* ai) { return new AttumenPositionAction(ai); }
    
    // Moroes
    static Action* moroes_focus_adds(PlayerbotAI* ai) { return new MoroesFocusAddsAction(ai); }
    static Action* moroes_position(PlayerbotAI* ai) { return new MoroesPositionAction(ai); }
    static Action* moroes_crowd_control(PlayerbotAI* ai) { return new MoroesCrowdControlAction(ai); }
    static Action* moroes_tank_swap(PlayerbotAI* ai) { return new MoroesTankSwapAction(ai); }
    
    // Maiden of Virtue
    static Action* maiden_repentance(PlayerbotAI* ai) { return new MaidenRepentanceAction(ai); }
    static Action* maiden_holy_ground(PlayerbotAI* ai) { return new MaidenHolyGroundAction(ai); }
    
    // Opera Event
    static Action* opera_position(PlayerbotAI* ai) { return new OperaPositionAction(ai); }
    static Action* opera_focus_target(PlayerbotAI* ai) { return new OperaFocusTargetAction(ai); }
    
    // Curator
    static Action* curator_flare(PlayerbotAI* ai) { return new CuratorFlareAction(ai); }
    static Action* curator_evocation(PlayerbotAI* ai) { return new CuratorEvocationAction(ai); }
    
    // Shade of Aran
    static Action* aran_flame_wreath(PlayerbotAI* ai) { return new AranFlameWreathAction(ai); }
    static Action* aran_blizzard(PlayerbotAI* ai) { return new AranBlizzardAction(ai); }
    static Action* aran_dragons_breath(PlayerbotAI* ai) { return new AranDragonsBreathAction(ai); }
    
    // Terestian Illhoof
    static Action* illhoof_demon_chains(PlayerbotAI* ai) { return new IllhoofDemonChainsAction(ai); }
    static Action* illhoof_imps(PlayerbotAI* ai) { return new IllhoofImpsAction(ai); }
    
    // Netherspite
    static Action* netherspite_beam(PlayerbotAI* ai) { return new NetherspiteBeamAction(ai); }
    static Action* netherspite_void_zone(PlayerbotAI* ai) { return new NetherspiteVoidZoneAction(ai); }
    
    // Prince Malchezaar
    static Action* malchezaar_infernal(PlayerbotAI* ai) { return new MalchezaarInfernalAction(ai); }
    static Action* malchezaar_enfeeble(PlayerbotAI* ai) { return new MalchezaarEnfeebleAction(ai); }
    
    // Nightbane
    static Action* nightbane_position(PlayerbotAI* ai) { return new NightbanePositionAction(ai); }
    static Action* nightbane_charred_earth(PlayerbotAI* ai) { return new NightbaneCharredEarthAction(ai); }
    static Action* nightbane_air_phase(PlayerbotAI* ai) { return new NightbaneAirPhaseAction(ai); }
    
    // Chess Event
    static Action* chess_event_move(PlayerbotAI* ai) { return new ChessEventMoveAction(ai); }
    static Action* chess_event_ability(PlayerbotAI* ai) { return new ChessEventAbilityAction(ai); }
    
    // Utility Actions
    static Action* karazhan_interrupt_rotation(PlayerbotAI* ai) { return new KarazhanInterruptRotationAction(ai); }
    static Action* karazhan_dispel(PlayerbotAI* ai) { return new KarazhanDispelAction(ai); }
};

#endif