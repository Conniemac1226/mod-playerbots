#ifndef _PLAYERBOT_BLACKMORASSVALUECONTEXT_H
#define _PLAYERBOT_BLACKMORASSVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "BlackMorassValues.h"

class BlackMorassValueContext : public NamedObjectContext<UntypedValue>
{
public:
    BlackMorassValueContext()
    {
        // Portal/Add Management Values
        creators["portal add active"] = &BlackMorassValueContext::portal_add_active;
        creators["medivh needs protection"] = &BlackMorassValueContext::medivh_needs_protection;
        
        // Aeonus Values
        creators["aeonus cleave danger"] = &BlackMorassValueContext::aeonus_cleave_danger;
        creators["aeonus engaged"] = &BlackMorassValueContext::aeonus_engaged;
        creators["sand breath danger"] = &BlackMorassValueContext::sand_breath_danger;
        creators["time stop active"] = &BlackMorassValueContext::time_stop_active;
        
        // Chrono Lord Deja Values
        creators["time lapse danger"] = &BlackMorassValueContext::time_lapse_danger;
        creators["arcane discharge danger"] = &BlackMorassValueContext::arcane_discharge_danger;
        creators["attraction active"] = &BlackMorassValueContext::attraction_active;
        
        // Temporus Values
        creators["wing buffet danger"] = &BlackMorassValueContext::wing_buffet_danger;
        creators["mortal wound active"] = &BlackMorassValueContext::mortal_wound_active;
        creators["temporus reflect active"] = &BlackMorassValueContext::temporus_reflect_active;
    }

private:
    // Portal/Add Management Value Factories
    static UntypedValue* portal_add_active(PlayerbotAI* botAI) { return new PortalAddActiveValue(botAI); }
    static UntypedValue* medivh_needs_protection(PlayerbotAI* botAI) { return new MedivhNeedsProtectionValue(botAI); }
    
    // Aeonus Value Factories
    static UntypedValue* aeonus_cleave_danger(PlayerbotAI* botAI) { return new AeonusCleaveDangerValue(botAI); }
    static UntypedValue* aeonus_engaged(PlayerbotAI* botAI) { return new AeonusEngagedValue(botAI); }
    static UntypedValue* sand_breath_danger(PlayerbotAI* botAI) { return new SandBreathDangerValue(botAI); }
    static UntypedValue* time_stop_active(PlayerbotAI* botAI) { return new TimeStopActiveValue(botAI); }
    
    // Chrono Lord Deja Value Factories
    static UntypedValue* time_lapse_danger(PlayerbotAI* botAI) { return new TimeLapseDangerValue(botAI); }
    static UntypedValue* arcane_discharge_danger(PlayerbotAI* botAI) { return new ArcaneDischargeValue(botAI); }
    static UntypedValue* attraction_active(PlayerbotAI* botAI) { return new AttractionActiveValue(botAI); }
    
    // Temporus Value Factories
    static UntypedValue* wing_buffet_danger(PlayerbotAI* botAI) { return new WingBuffetDangerValue(botAI); }
    static UntypedValue* mortal_wound_active(PlayerbotAI* botAI) { return new MortalWoundActiveValue(botAI); }
    static UntypedValue* temporus_reflect_active(PlayerbotAI* botAI) { return new TemporusReflectActiveValue(botAI); }
};

#endif