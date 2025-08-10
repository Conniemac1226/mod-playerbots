#ifndef _PLAYERBOT_ARCATRAZACTIONCONTEXT_H
#define _PLAYERBOT_ARCATRAZACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "ArcatrazActions.h"

class ArcatrazActionContext : public NamedObjectContext<Action>
{
public:
    ArcatrazActionContext()
    {
        creators["avoid void zone"] = &ArcatrazActionContext::avoid_void_zone;
        creators["avoid shadow nova"] = &ArcatrazActionContext::avoid_shadow_nova;
        creators["seed of corruption dispel"] = &ArcatrazActionContext::seed_of_corruption_dispel;
        creators["dalliah whirlwind"] = &ArcatrazActionContext::dalliah_whirlwind;
        creators["dalliah heal interrupt"] = &ArcatrazActionContext::dalliah_heal_interrupt;
        creators["soccothrates knock away"] = &ArcatrazActionContext::soccothrates_knock_away;
        creators["soccothrates charge"] = &ArcatrazActionContext::soccothrates_charge;
        creators["skyriss illusion"] = &ArcatrazActionContext::skyriss_illusion;
        creators["skyriss fear"] = &ArcatrazActionContext::skyriss_fear;
        creators["skyriss domination"] = &ArcatrazActionContext::skyriss_domination;
    }

private:
    static Action* avoid_void_zone(PlayerbotAI* botAI) { return new AvoidVoidZoneAction(botAI); }
    static Action* avoid_shadow_nova(PlayerbotAI* botAI) { return new AvoidShadowNovaAction(botAI); }
    static Action* seed_of_corruption_dispel(PlayerbotAI* botAI) { return new SeedOfCorruptionDispelAction(botAI); }
    static Action* dalliah_whirlwind(PlayerbotAI* botAI) { return new DalliahWhirlwindAction(botAI); }
    static Action* dalliah_heal_interrupt(PlayerbotAI* botAI) { return new DalliahHealInterruptAction(botAI); }
    static Action* soccothrates_knock_away(PlayerbotAI* botAI) { return new SoccothratesKnockAwayAction(botAI); }
    static Action* soccothrates_charge(PlayerbotAI* botAI) { return new SoccothratesChargeAction(botAI); }
    static Action* skyriss_illusion(PlayerbotAI* botAI) { return new SkyrissIllusionAction(botAI); }
    static Action* skyriss_fear(PlayerbotAI* botAI) { return new SkyrissFearAction(botAI); }
    static Action* skyriss_domination(PlayerbotAI* botAI) { return new SkyrissDominationAction(botAI); }
};

#endif