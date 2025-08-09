#pragma once
#include "NamedObjectContext.h"
#include "BlackMorassActions.h"

class BlackMorassActionContext : public NamedObjectContext<Action>
{
public:
    BlackMorassActionContext()
    {
        creators["aeonus avoid cleave"] = &BlackMorassActionContext::aeonus_avoid_cleave;
        creators["aeonus position"] = &BlackMorassActionContext::aeonus_position;
        creators["avoid time lapse"] = &BlackMorassActionContext::avoid_time_lapse;
        creators["avoid arcane discharge"] = &BlackMorassActionContext::avoid_arcane_discharge;
        creators["avoid wing buffet"] = &BlackMorassActionContext::avoid_wing_buffet;
    }

private:
    static Action* aeonus_avoid_cleave(PlayerbotAI* botAI) { return new AeonusAvoidCleaveAction(botAI); }
    static Action* aeonus_position(PlayerbotAI* botAI) { return new AeonusPositionAction(botAI); }
    static Action* avoid_time_lapse(PlayerbotAI* botAI) { return new AvoidTimeLapseAction(botAI); }
    static Action* avoid_arcane_discharge(PlayerbotAI* botAI) { return new AvoidArcaneDischargeAction(botAI); }
    static Action* avoid_wing_buffet(PlayerbotAI* botAI) { return new AvoidWingBuffetAction(botAI); }
};