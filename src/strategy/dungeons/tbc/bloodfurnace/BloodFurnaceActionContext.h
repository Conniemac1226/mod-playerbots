#ifndef _PLAYERBOT_BLOODFURNACEACTIONCONTEXT_H
#define _PLAYERBOT_BLOODFURNACEACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "BloodFurnaceActions.h"

class BloodFurnaceActionContext : public NamedObjectContext<Action>
{
public:
    BloodFurnaceActionContext()
    {
        // The Maker actions
        creators["avoid exploding beaker"] = &BloodFurnaceActionContext::avoid_exploding_beaker;
        creators["break domination"] = &BloodFurnaceActionContext::break_domination;
        
        // Broggok actions
        creators["avoid poison cloud"] = &BloodFurnaceActionContext::avoid_poison_cloud;
        creators["interrupt poison bolt"] = &BloodFurnaceActionContext::interrupt_poison_bolt;
        creators["avoid slime spray"] = &BloodFurnaceActionContext::avoid_slime_spray;
        
        // Kelidan actions
        creators["attack shadowmoon channeler"] = &BloodFurnaceActionContext::attack_shadowmoon_channeler;
        creators["avoid burning nova"] = &BloodFurnaceActionContext::avoid_burning_nova;
        creators["interrupt shadow bolt volley"] = &BloodFurnaceActionContext::interrupt_shadow_bolt_volley;
        creators["avoid vortex"] = &BloodFurnaceActionContext::avoid_vortex;
    }

private:
    // The Maker action creators
    static Action* avoid_exploding_beaker(PlayerbotAI* ai) { return new MakerExplodingBeakerAction(ai); }
    static Action* break_domination(PlayerbotAI* ai) { return new MakerDominationAction(ai); }
    
    // Broggok action creators
    static Action* avoid_poison_cloud(PlayerbotAI* ai) { return new BroggokAvoidPoisonCloudAction(ai); }
    static Action* interrupt_poison_bolt(PlayerbotAI* ai) { return new BroggokInterruptPoisonBoltAction(ai); }
    static Action* avoid_slime_spray(PlayerbotAI* ai) { return new BroggokAvoidSlimeSprayAction(ai); }
    
    // Kelidan action creators
    static Action* attack_shadowmoon_channeler(PlayerbotAI* ai) { return new AttackShadowmoonChannelerAction(ai); }
    static Action* avoid_burning_nova(PlayerbotAI* ai) { return new KelidanBurningNovaAction(ai); }
    static Action* interrupt_shadow_bolt_volley(PlayerbotAI* ai) { return new KelidanInterruptShadowBoltVolleyAction(ai); }
    static Action* avoid_vortex(PlayerbotAI* ai) { return new KelidanAvoidVortexAction(ai); }
};

#endif