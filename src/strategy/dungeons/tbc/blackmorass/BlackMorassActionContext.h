#pragma once
#include "NamedObjectContext.h"
#include "BlackMorassActions.h"

class BlackMorassActionContext : public NamedObjectContext<Action>
{
public:
    BlackMorassActionContext()
    {
        // Portal/Add Management
        creators["attack portal add"] = &BlackMorassActionContext::attack_portal_add;
        creators["protect medivh"] = &BlackMorassActionContext::protect_medivh;
        
        // Aeonus
        creators["aeonus avoid cleave"] = &BlackMorassActionContext::aeonus_avoid_cleave;
        creators["aeonus position"] = &BlackMorassActionContext::aeonus_position;
        creators["avoid sand breath"] = &BlackMorassActionContext::avoid_sand_breath;
        creators["handle time stop"] = &BlackMorassActionContext::handle_time_stop;
        creators["handle aeonus enrage"] = &BlackMorassActionContext::handle_aeonus_enrage;
        
        // Chrono Lord Deja
        creators["avoid time lapse"] = &BlackMorassActionContext::avoid_time_lapse;
        creators["avoid arcane discharge"] = &BlackMorassActionContext::avoid_arcane_discharge;
        creators["handle attraction"] = &BlackMorassActionContext::handle_attraction;
        creators["interrupt arcane blast"] = &BlackMorassActionContext::interrupt_arcane_blast;
        
        // Temporus
        creators["avoid wing buffet"] = &BlackMorassActionContext::avoid_wing_buffet;
        creators["heal mortal wound"] = &BlackMorassActionContext::heal_mortal_wound;
        creators["stop casting reflect"] = &BlackMorassActionContext::stop_casting_reflect;
        creators["dispel hasten"] = &BlackMorassActionContext::dispel_hasten;
    }

private:
    // Portal/Add Management
    static Action* attack_portal_add(PlayerbotAI* botAI) { return new AttackPortalAddAction(botAI); }
    static Action* protect_medivh(PlayerbotAI* botAI) { return new ProtectMedivhAction(botAI); }
    
    // Aeonus
    static Action* aeonus_avoid_cleave(PlayerbotAI* botAI) { return new AeonusAvoidCleaveAction(botAI); }
    static Action* aeonus_position(PlayerbotAI* botAI) { return new AeonusPositionAction(botAI); }
    static Action* avoid_sand_breath(PlayerbotAI* botAI) { return new AeonusSandBreathAction(botAI); }
    static Action* handle_time_stop(PlayerbotAI* botAI) { return new AeonusTimeStopAction(botAI); }
    static Action* handle_aeonus_enrage(PlayerbotAI* botAI) { return new AeonusEnrageAction(botAI); }
    
    // Chrono Lord Deja
    static Action* avoid_time_lapse(PlayerbotAI* botAI) { return new AvoidTimeLapseAction(botAI); }
    static Action* avoid_arcane_discharge(PlayerbotAI* botAI) { return new AvoidArcaneDischargeAction(botAI); }
    static Action* handle_attraction(PlayerbotAI* botAI) { return new DejaAttractionAction(botAI); }
    static Action* interrupt_arcane_blast(PlayerbotAI* botAI) { return new DejaArcaneBlastInterruptAction(botAI); }
    
    // Temporus
    static Action* avoid_wing_buffet(PlayerbotAI* botAI) { return new AvoidWingBuffetAction(botAI); }
    static Action* heal_mortal_wound(PlayerbotAI* botAI) { return new TemporusMortalWoundAction(botAI); }
    static Action* stop_casting_reflect(PlayerbotAI* botAI) { return new TemporusReflectAction(botAI); }
    static Action* dispel_hasten(PlayerbotAI* botAI) { return new TemporusHastenDispelAction(botAI); }
};