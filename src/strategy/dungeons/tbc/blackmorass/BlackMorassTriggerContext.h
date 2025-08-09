#pragma once
#include "NamedObjectContext.h"
#include "BlackMorassTriggers.h"

class BlackMorassTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackMorassTriggerContext()
    {
        creators["aeonus cleave nearby"] = &BlackMorassTriggerContext::aeonus_cleave_nearby;
        creators["aeonus engaged"] = &BlackMorassTriggerContext::aeonus_engaged;
        creators["chrono lord deja engaged"] = &BlackMorassTriggerContext::chrono_lord_deja_engaged;
        creators["temporus engaged"] = &BlackMorassTriggerContext::temporus_engaged;
        creators["time lapse nearby"] = &BlackMorassTriggerContext::time_lapse_nearby;
        creators["arcane discharge nearby"] = &BlackMorassTriggerContext::arcane_discharge_nearby;
        creators["wing buffet nearby"] = &BlackMorassTriggerContext::wing_buffet_nearby;
    }

private:
    static Trigger* aeonus_cleave_nearby(PlayerbotAI* botAI) { return new AeonusCleaveNearbyTrigger(botAI); }
    static Trigger* aeonus_engaged(PlayerbotAI* botAI) { return new AeonusEngagedTrigger(botAI); }
    static Trigger* chrono_lord_deja_engaged(PlayerbotAI* botAI) { return new ChronoLordDejaEngagedTrigger(botAI); }
    static Trigger* temporus_engaged(PlayerbotAI* botAI) { return new TemporusEngagedTrigger(botAI); }
    static Trigger* time_lapse_nearby(PlayerbotAI* botAI) { return new TimeLapseNearbyTrigger(botAI); }
    static Trigger* arcane_discharge_nearby(PlayerbotAI* botAI) { return new ArcaneDischargeNearbyTrigger(botAI); }
    static Trigger* wing_buffet_nearby(PlayerbotAI* botAI) { return new WingBuffetNearbyTrigger(botAI); }
};