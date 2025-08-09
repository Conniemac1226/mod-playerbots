#ifndef _PLAYERBOT_BLOODFURNACETRIGGERCONTEXT_H
#define _PLAYERBOT_BLOODFURNACETRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "BloodFurnaceTriggers.h"

class BloodFurnaceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BloodFurnaceTriggerContext()
    {
        // The Maker triggers
        creators["the maker exploding beaker"] = &BloodFurnaceTriggerContext::the_maker_exploding_beaker;
        creators["the maker domination"] = &BloodFurnaceTriggerContext::the_maker_domination;
        
        // Broggok triggers
        creators["broggok poison cloud"] = &BloodFurnaceTriggerContext::broggok_poison_cloud;
        creators["broggok interrupt poison bolt"] = &BloodFurnaceTriggerContext::broggok_interrupt_poison_bolt;
        creators["broggok slime spray"] = &BloodFurnaceTriggerContext::broggok_slime_spray;
        
        // Kelidan triggers
        creators["kelidan shadowmoon channeler"] = &BloodFurnaceTriggerContext::kelidan_shadowmoon_channeler;
        creators["kelidan burning nova"] = &BloodFurnaceTriggerContext::kelidan_burning_nova;
        creators["kelidan interrupt shadow bolt volley"] = &BloodFurnaceTriggerContext::kelidan_interrupt_shadow_bolt_volley;
        creators["kelidan vortex"] = &BloodFurnaceTriggerContext::kelidan_vortex;
    }

private:
    // The Maker trigger creators
    static Trigger* the_maker_exploding_beaker(PlayerbotAI* ai) { return new TheMakerExplodingBeakerTrigger(ai); }
    static Trigger* the_maker_domination(PlayerbotAI* ai) { return new TheMakerDominationTrigger(ai); }
    
    // Broggok trigger creators
    static Trigger* broggok_poison_cloud(PlayerbotAI* ai) { return new BroggokPoisonCloudTrigger(ai); }
    static Trigger* broggok_interrupt_poison_bolt(PlayerbotAI* ai) { return new BroggokInterruptPoisonBoltTrigger(ai); }
    static Trigger* broggok_slime_spray(PlayerbotAI* ai) { return new BroggokSlimeSprayTrigger(ai); }
    
    // Kelidan trigger creators
    static Trigger* kelidan_shadowmoon_channeler(PlayerbotAI* ai) { return new KelidanShadowmoonChannelerTrigger(ai); }
    static Trigger* kelidan_burning_nova(PlayerbotAI* ai) { return new KelidanBurningNovaTrigger(ai); }
    static Trigger* kelidan_interrupt_shadow_bolt_volley(PlayerbotAI* ai) { return new KelidanInterruptShadowBoltVolleyTrigger(ai); }
    static Trigger* kelidan_vortex(PlayerbotAI* ai) { return new KelidanVortexTrigger(ai); }
};

#endif