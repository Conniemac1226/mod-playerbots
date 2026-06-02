#pragma once

#include "BlackMorassTriggers.h"
#include "NamedObjectContext.h"

class BlackMorassTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackMorassTriggerContext()
    {
        creators["portal add active"] = &BlackMorassTriggerContext::portal_add_active;
        creators["medivh needs protection"] = &BlackMorassTriggerContext::medivh_needs_protection;

        creators["aeonus cleave danger"] = &BlackMorassTriggerContext::aeonus_cleave_nearby;
        creators["aeonus engaged"] = &BlackMorassTriggerContext::aeonus_engaged;
        creators["sand breath danger"] = &BlackMorassTriggerContext::sand_breath_danger;

        creators["chrono lord deja engaged"] = &BlackMorassTriggerContext::chrono_lord_deja_engaged;
        creators["time lapse danger"] = &BlackMorassTriggerContext::time_lapse_danger;
        creators["arcane discharge danger"] = &BlackMorassTriggerContext::arcane_discharge_danger;
        creators["attraction active"] = &BlackMorassTriggerContext::attraction_active;
        creators["deja arcane blast casting"] = &BlackMorassTriggerContext::deja_arcane_blast_casting;

        creators["temporus engaged"] = &BlackMorassTriggerContext::temporus_engaged;
        creators["wing buffet danger"] = &BlackMorassTriggerContext::wing_buffet_danger;
        creators["mortal wound active"] = &BlackMorassTriggerContext::mortal_wound_active;
        creators["temporus reflect active"] = &BlackMorassTriggerContext::temporus_reflect_active;
        creators["temporus hasten active"] = &BlackMorassTriggerContext::temporus_hasten_active;
    }

private:
    static Trigger* portal_add_active(PlayerbotAI* botAI) { return new PortalAddActiveTrigger(botAI); }
    static Trigger* medivh_needs_protection(PlayerbotAI* botAI) { return new MedivhNeedsProtectionTrigger(botAI); }

    static Trigger* aeonus_cleave_nearby(PlayerbotAI* botAI) { return new AeonusCleaveNearbyTrigger(botAI); }
    static Trigger* aeonus_engaged(PlayerbotAI* botAI) { return new AeonusEngagedTrigger(botAI); }
    static Trigger* sand_breath_danger(PlayerbotAI* botAI) { return new SandBreathDangerTrigger(botAI); }

    static Trigger* chrono_lord_deja_engaged(PlayerbotAI* botAI) { return new ChronoLordDejaEngagedTrigger(botAI); }
    static Trigger* time_lapse_danger(PlayerbotAI* botAI) { return new TimeLapseDangerTrigger(botAI); }
    static Trigger* arcane_discharge_danger(PlayerbotAI* botAI) { return new ArcaneDischargeDangerTrigger(botAI); }
    static Trigger* attraction_active(PlayerbotAI* botAI) { return new AttractionActiveTrigger(botAI); }
    static Trigger* deja_arcane_blast_casting(PlayerbotAI* botAI) { return new DejaArcaneBlastCastingTrigger(botAI); }

    static Trigger* temporus_engaged(PlayerbotAI* botAI) { return new TemporusEngagedTrigger(botAI); }
    static Trigger* wing_buffet_danger(PlayerbotAI* botAI) { return new WingBuffetDangerTrigger(botAI); }
    static Trigger* mortal_wound_active(PlayerbotAI* botAI) { return new MortalWoundActiveTrigger(botAI); }
    static Trigger* temporus_reflect_active(PlayerbotAI* botAI) { return new TemporusReflectActiveTrigger(botAI); }
    static Trigger* temporus_hasten_active(PlayerbotAI* botAI) { return new TemporusHastenActiveTrigger(botAI); }
};
