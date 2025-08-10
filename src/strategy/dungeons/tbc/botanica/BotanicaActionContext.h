#ifndef _PLAYERBOT_BOTANICAACTIONCONTEXT_H
#define _PLAYERBOT_BOTANICAACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "BotanicaActions.h"

class BotanicaActionContext : public NamedObjectContext<Action>
{
public:
    BotanicaActionContext()
    {
        creators["sarannis resonance dispel"] = &BotanicaActionContext::sarannis_resonance_dispel;
        creators["sarannis reinforcements"] = &BotanicaActionContext::sarannis_reinforcements;
        creators["freywinn frayer priority"] = &BotanicaActionContext::freywinn_frayer_priority;
        creators["freywinn tranquility"] = &BotanicaActionContext::freywinn_tranquility;
        creators["laj allergic reaction"] = &BotanicaActionContext::laj_allergic_reaction;
        creators["laj teleport position"] = &BotanicaActionContext::laj_teleport_position;
        creators["thorngrin sacrifice"] = &BotanicaActionContext::thorngrin_sacrifice;
        creators["thorngrin hellfire"] = &BotanicaActionContext::thorngrin_hellfire;
        creators["warp splinter war stomp"] = &BotanicaActionContext::warp_splinter_war_stomp;
        creators["warp splinter arcane volley"] = &BotanicaActionContext::warp_splinter_arcane_volley;
    }

private:
    static Action* sarannis_resonance_dispel(PlayerbotAI* botAI) { return new SarannisResonanceDispelAction(botAI); }
    static Action* sarannis_reinforcements(PlayerbotAI* botAI) { return new SarannisReinforcementsAction(botAI); }
    static Action* freywinn_frayer_priority(PlayerbotAI* botAI) { return new FreywinnFrayerPriorityAction(botAI); }
    static Action* freywinn_tranquility(PlayerbotAI* botAI) { return new FreywinnTranquilityAction(botAI); }
    static Action* laj_allergic_reaction(PlayerbotAI* botAI) { return new LajAllergicReactionAction(botAI); }
    static Action* laj_teleport_position(PlayerbotAI* botAI) { return new LajTeleportPositionAction(botAI); }
    static Action* thorngrin_sacrifice(PlayerbotAI* botAI) { return new ThorngrinSacrificeAction(botAI); }
    static Action* thorngrin_hellfire(PlayerbotAI* botAI) { return new ThorngrinHellfireAction(botAI); }
    static Action* warp_splinter_war_stomp(PlayerbotAI* botAI) { return new WarpSplinterWarStompAction(botAI); }
    static Action* warp_splinter_arcane_volley(PlayerbotAI* botAI) { return new WarpSplinterArcaneVolleyAction(botAI); }
};

#endif