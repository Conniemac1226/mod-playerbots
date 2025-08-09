#pragma once
#include "NamedObjectContext.h"
#include "EscapeFromDurnholdeActions.h"

class EscapeFromDurnholdeActionContext : public NamedObjectContext<Action>
{
public:
    EscapeFromDurnholdeActionContext()
    {
        creators["heal thrall"] = &EscapeFromDurnholdeActionContext::heal_thrall;
        creators["thrall emergency heal"] = &EscapeFromDurnholdeActionContext::thrall_emergency_heal;
        creators["efd avoid whirlwind"] = &EscapeFromDurnholdeActionContext::efd_avoid_whirlwind;
        creators["efd return position"] = &EscapeFromDurnholdeActionContext::efd_return_position;
        creators["dispel hammer of justice"] = &EscapeFromDurnholdeActionContext::dispel_hammer_of_justice;
        creators["epoch hunter position"] = &EscapeFromDurnholdeActionContext::epoch_hunter_position;
        creators["cancel magic disruption"] = &EscapeFromDurnholdeActionContext::cancel_magic_disruption;
    }

private:
    static Action* heal_thrall(PlayerbotAI* ai) { return new HealThrallAction(ai); }
    static Action* thrall_emergency_heal(PlayerbotAI* ai) { return new ThrallEmergencyHealAction(ai); }
    static Action* efd_avoid_whirlwind(PlayerbotAI* ai) { return new EfdAvoidWhirlwindAction(ai); }
    static Action* efd_return_position(PlayerbotAI* ai) { return new EfdReturnPositionAction(ai); }
    static Action* dispel_hammer_of_justice(PlayerbotAI* ai) { return new DispelHammerOfJusticeAction(ai); }
    static Action* epoch_hunter_position(PlayerbotAI* ai) { return new EpochHunterPositionAction(ai); }
    static Action* cancel_magic_disruption(PlayerbotAI* ai) { return new CancelMagicDisruptionAction(ai); }
};