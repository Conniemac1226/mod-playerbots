#ifndef _PLAYERBOT_ARCATRAZTRIGGERCONTEXT_H
#define _PLAYERBOT_ARCATRAZTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ArcatrazTriggers.h"

class ArcatrazTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ArcatrazTriggerContext()
    {
        creators["zereketh void zone"] = &ArcatrazTriggerContext::zereketh_void_zone;
        creators["zereketh shadow nova"] = &ArcatrazTriggerContext::zereketh_shadow_nova;
        creators["zereketh seed of corruption"] = &ArcatrazTriggerContext::zereketh_seed_of_corruption;
        creators["dalliah whirlwind"] = &ArcatrazTriggerContext::dalliah_whirlwind;
        creators["dalliah heal"] = &ArcatrazTriggerContext::dalliah_heal;
        creators["soccothrates knock away"] = &ArcatrazTriggerContext::soccothrates_knock_away;
        creators["soccothrates charge"] = &ArcatrazTriggerContext::soccothrates_charge;
        creators["felfire ground"] = &ArcatrazTriggerContext::felfire_ground;
        creators["mellichar immune"] = &ArcatrazTriggerContext::mellichar_immune;
        creators["mellichar adds active"] = &ArcatrazTriggerContext::mellichar_adds_active;
        creators["skyriss illusion"] = &ArcatrazTriggerContext::skyriss_illusion;
        creators["skyriss fear"] = &ArcatrazTriggerContext::skyriss_fear;
        creators["skyriss domination"] = &ArcatrazTriggerContext::skyriss_domination;
    }

private:
    static Trigger* zereketh_void_zone(PlayerbotAI* botAI) { return new ZerekethVoidZoneTrigger(botAI); }
    static Trigger* zereketh_shadow_nova(PlayerbotAI* botAI) { return new ZerekethShadowNovaTrigger(botAI); }
    static Trigger* zereketh_seed_of_corruption(PlayerbotAI* botAI) { return new ZerekethSeedOfCorruptionTrigger(botAI); }
    static Trigger* dalliah_whirlwind(PlayerbotAI* botAI) { return new DalliahWhirlwindTrigger(botAI); }
    static Trigger* dalliah_heal(PlayerbotAI* botAI) { return new DalliahHealTrigger(botAI); }
    static Trigger* soccothrates_knock_away(PlayerbotAI* botAI) { return new SoccothratesKnockAwayTrigger(botAI); }
    static Trigger* soccothrates_charge(PlayerbotAI* botAI) { return new SoccothratesChargeTrigger(botAI); }
    static Trigger* felfire_ground(PlayerbotAI* botAI) { return new FelfireGroundTrigger(botAI); }
    static Trigger* mellichar_immune(PlayerbotAI* botAI) { return new MellicharImmuneTrigger(botAI); }
    static Trigger* mellichar_adds_active(PlayerbotAI* botAI) { return new MellicharAddsActiveTrigger(botAI); }
    static Trigger* skyriss_illusion(PlayerbotAI* botAI) { return new SkyrissIllusionTrigger(botAI); }
    static Trigger* skyriss_fear(PlayerbotAI* botAI) { return new SkyrissFearTrigger(botAI); }
    static Trigger* skyriss_domination(PlayerbotAI* botAI) { return new SkyrissDominationTrigger(botAI); }
};

#endif
