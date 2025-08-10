#ifndef _PLAYERBOT_BOTANICATRIGGERCONTEXT_H
#define _PLAYERBOT_BOTANICATRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "BotanicaTriggers.h"

class BotanicaTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BotanicaTriggerContext()
    {
        creators["sarannis resonance"] = &BotanicaTriggerContext::sarannis_resonance;
        creators["sarannis reinforcements"] = &BotanicaTriggerContext::sarannis_reinforcements;
        creators["freywinn frayers"] = &BotanicaTriggerContext::freywinn_frayers;
        creators["freywinn tranquility"] = &BotanicaTriggerContext::freywinn_tranquility;
        creators["laj allergic reaction"] = &BotanicaTriggerContext::laj_allergic_reaction;
        creators["laj teleport"] = &BotanicaTriggerContext::laj_teleport;
        creators["thorngrin sacrifice"] = &BotanicaTriggerContext::thorngrin_sacrifice;
        creators["thorngrin hellfire"] = &BotanicaTriggerContext::thorngrin_hellfire;
        creators["warp splinter war stomp"] = &BotanicaTriggerContext::warp_splinter_war_stomp;
        creators["warp splinter arcane volley"] = &BotanicaTriggerContext::warp_splinter_arcane_volley;
    }

private:
    static Trigger* sarannis_resonance(PlayerbotAI* botAI) { return new SarannisResonanceTrigger(botAI); }
    static Trigger* sarannis_reinforcements(PlayerbotAI* botAI) { return new SarannisReinforcementsTrigger(botAI); }
    static Trigger* freywinn_frayers(PlayerbotAI* botAI) { return new FreywinnFrayersTrigger(botAI); }
    static Trigger* freywinn_tranquility(PlayerbotAI* botAI) { return new FreywinnTranquilityTrigger(botAI); }
    static Trigger* laj_allergic_reaction(PlayerbotAI* botAI) { return new LajAllergicReactionTrigger(botAI); }
    static Trigger* laj_teleport(PlayerbotAI* botAI) { return new LajTeleportTrigger(botAI); }
    static Trigger* thorngrin_sacrifice(PlayerbotAI* botAI) { return new ThorngrinSacrificeTrigger(botAI); }
    static Trigger* thorngrin_hellfire(PlayerbotAI* botAI) { return new ThorngrinHellfireTrigger(botAI); }
    static Trigger* warp_splinter_war_stomp(PlayerbotAI* botAI) { return new WarpSplinterWarStompTrigger(botAI); }
    static Trigger* warp_splinter_arcane_volley(PlayerbotAI* botAI) { return new WarpSplinterArcaneVolleyTrigger(botAI); }
};

#endif