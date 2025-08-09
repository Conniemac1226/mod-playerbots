#pragma once
#include "NamedObjectContext.h"
#include "EscapeFromDurnholdeTriggers.h"

class EscapeFromDurnholdeTriggerContext : public NamedObjectContext<Trigger>
{
public:
    EscapeFromDurnholdeTriggerContext()
    {
        creators["thrall low health"] = &EscapeFromDurnholdeTriggerContext::thrall_low_health;
        creators["thrall critical health"] = &EscapeFromDurnholdeTriggerContext::thrall_critical_health;
        creators["lieutenant drake whirlwind"] = &EscapeFromDurnholdeTriggerContext::lieutenant_drake_whirlwind;
        creators["efd return position"] = &EscapeFromDurnholdeTriggerContext::efd_return_position;
        creators["captain skarloc hammer of justice"] = &EscapeFromDurnholdeTriggerContext::captain_skarloc_hammer_of_justice;
        creators["epoch hunter sand breath"] = &EscapeFromDurnholdeTriggerContext::epoch_hunter_sand_breath;
        creators["epoch hunter magic disruption aura"] = &EscapeFromDurnholdeTriggerContext::epoch_hunter_magic_disruption_aura;
    }

private:
    static Trigger* thrall_low_health(PlayerbotAI* ai) { return new ThrallLowHealthTrigger(ai); }
    static Trigger* thrall_critical_health(PlayerbotAI* ai) { return new ThrallCriticalHealthTrigger(ai); }
    static Trigger* lieutenant_drake_whirlwind(PlayerbotAI* ai) { return new LieutenantDrakeWhirlwindTrigger(ai); }
    static Trigger* efd_return_position(PlayerbotAI* ai) { return new EfdReturnPositionTrigger(ai); }
    static Trigger* captain_skarloc_hammer_of_justice(PlayerbotAI* ai) { return new CaptainSkarlocHammerOfJusticeTrigger(ai); }
    static Trigger* epoch_hunter_sand_breath(PlayerbotAI* ai) { return new EpochHunterSandBreathTrigger(ai); }
    static Trigger* epoch_hunter_magic_disruption_aura(PlayerbotAI* ai) { return new EpochHunterMagicDisruptionAuraTrigger(ai); }
};