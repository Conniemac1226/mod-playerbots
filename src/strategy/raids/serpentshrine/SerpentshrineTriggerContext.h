#ifndef _PLAYERBOT_SERPENTSHRINETRIGGERCONTEXT_H
#define _PLAYERBOT_SERPENTSHRINETRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "SerpentshrineTriggers.h"

class SerpentshrineTriggerContext : public NamedObjectContext<Trigger>
{
public:
    SerpentshrineTriggerContext()
    {
        creators["hydross mark of hydross"] = &SerpentshrineTriggerContext::hydross_mark_of_hydross;
        creators["hydross mark of corruption"] = &SerpentshrineTriggerContext::hydross_mark_of_corruption;
        creators["hydross water tomb"] = &SerpentshrineTriggerContext::hydross_water_tomb;
        creators["hydross vile sludge"] = &SerpentshrineTriggerContext::hydross_vile_sludge;
        creators["hydross adds"] = &SerpentshrineTriggerContext::hydross_adds;
        creators["hydross transition needed"] = &SerpentshrineTriggerContext::hydross_transition_needed;
        creators["hydross tank position"] = &SerpentshrineTriggerContext::hydross_tank_position;
        
        // The Lurker Below
        creators["lurker spout"] = &SerpentshrineTriggerContext::lurker_spout;
        creators["lurker whirl"] = &SerpentshrineTriggerContext::lurker_whirl;
        creators["lurker geyser"] = &SerpentshrineTriggerContext::lurker_geyser;
        creators["lurker adds"] = &SerpentshrineTriggerContext::lurker_adds;
        creators["lurker position"] = &SerpentshrineTriggerContext::lurker_position;
        
        // Leotheras the Blind
        creators["leotheras whirlwind"] = &SerpentshrineTriggerContext::leotheras_whirlwind;
        creators["leotheras chaos blast"] = &SerpentshrineTriggerContext::leotheras_chaos_blast;
        creators["leotheras inner demon"] = &SerpentshrineTriggerContext::leotheras_inner_demon;
        creators["leotheras shadow"] = &SerpentshrineTriggerContext::leotheras_shadow;
        creators["leotheras position"] = &SerpentshrineTriggerContext::leotheras_position;
        
        // Fathom-Lord Karathress
        creators["karathress cataclysmic bolt"] = &SerpentshrineTriggerContext::karathress_cataclysmic_bolt;
        creators["karathress sear nova"] = &SerpentshrineTriggerContext::karathress_sear_nova;
        creators["karathress advisors"] = &SerpentshrineTriggerContext::karathress_advisors;
        creators["karathress tidal surge"] = &SerpentshrineTriggerContext::karathress_tidal_surge;
        creators["karathress totems"] = &SerpentshrineTriggerContext::karathress_totems;
        
        // Morogrim Tidewalker
        creators["morogrim tidal wave"] = &SerpentshrineTriggerContext::morogrim_tidal_wave;
        creators["morogrim watery grave"] = &SerpentshrineTriggerContext::morogrim_watery_grave;
        creators["morogrim murlocs"] = &SerpentshrineTriggerContext::morogrim_murlocs;
        creators["morogrim globules"] = &SerpentshrineTriggerContext::morogrim_globules;
        creators["morogrim position"] = &SerpentshrineTriggerContext::morogrim_position;
        
        // Lady Vashj
        creators["vashj shock blast"] = &SerpentshrineTriggerContext::vashj_shock_blast;
        creators["vashj static charge"] = &SerpentshrineTriggerContext::vashj_static_charge;
        creators["vashj entangle"] = &SerpentshrineTriggerContext::vashj_entangle;
        creators["vashj enchanted elemental"] = &SerpentshrineTriggerContext::vashj_enchanted_elemental;
        creators["vashj tainted elemental"] = &SerpentshrineTriggerContext::vashj_tainted_elemental;
        creators["vashj coilfang elite"] = &SerpentshrineTriggerContext::vashj_coilfang_elite;
        creators["vashj coilfang strider"] = &SerpentshrineTriggerContext::vashj_coilfang_strider;
        creators["vashj sporebat"] = &SerpentshrineTriggerContext::vashj_sporebat;
        creators["vashj position"] = &SerpentshrineTriggerContext::vashj_position;
        creators["vashj tainted core"] = &SerpentshrineTriggerContext::vashj_tainted_core;
    }

private:
    static Trigger* hydross_mark_of_hydross(PlayerbotAI* botAI) { return new HydrossMarkOfHydrossTrigger(botAI); }
    static Trigger* hydross_mark_of_corruption(PlayerbotAI* botAI) { return new HydrossMarkOfCorruptionTrigger(botAI); }
    static Trigger* hydross_water_tomb(PlayerbotAI* botAI) { return new HydrossWaterTombTrigger(botAI); }
    static Trigger* hydross_vile_sludge(PlayerbotAI* botAI) { return new HydrossVileSludgeTrigger(botAI); }
    static Trigger* hydross_adds(PlayerbotAI* botAI) { return new HydrossAddsTrigger(botAI); }
    static Trigger* hydross_transition_needed(PlayerbotAI* botAI) { return new HydrossTransitionNeededTrigger(botAI); }
    static Trigger* hydross_tank_position(PlayerbotAI* botAI) { return new HydrossTankPositionTrigger(botAI); }
    
    // The Lurker Below
    static Trigger* lurker_spout(PlayerbotAI* botAI) { return new LurkerSpoutTrigger(botAI); }
    static Trigger* lurker_whirl(PlayerbotAI* botAI) { return new LurkerWhirlTrigger(botAI); }
    static Trigger* lurker_geyser(PlayerbotAI* botAI) { return new LurkerGeyserTrigger(botAI); }
    static Trigger* lurker_adds(PlayerbotAI* botAI) { return new LurkerAddsTrigger(botAI); }
    static Trigger* lurker_position(PlayerbotAI* botAI) { return new LurkerPositionTrigger(botAI); }
    
    // Leotheras the Blind
    static Trigger* leotheras_whirlwind(PlayerbotAI* botAI) { return new LeotherasWhirlwindTrigger(botAI); }
    static Trigger* leotheras_chaos_blast(PlayerbotAI* botAI) { return new LeotherasChaosBlastTrigger(botAI); }
    static Trigger* leotheras_inner_demon(PlayerbotAI* botAI) { return new LeotherasInnerDemonTrigger(botAI); }
    static Trigger* leotheras_shadow(PlayerbotAI* botAI) { return new LeotherasShadowTrigger(botAI); }
    static Trigger* leotheras_position(PlayerbotAI* botAI) { return new LeotherasPositionTrigger(botAI); }
    
    // Fathom-Lord Karathress
    static Trigger* karathress_cataclysmic_bolt(PlayerbotAI* botAI) { return new KarathressCataclysmicBoltTrigger(botAI); }
    static Trigger* karathress_sear_nova(PlayerbotAI* botAI) { return new KarathressSearNovaTrigger(botAI); }
    static Trigger* karathress_advisors(PlayerbotAI* botAI) { return new KarathressAdvisorsTrigger(botAI); }
    static Trigger* karathress_tidal_surge(PlayerbotAI* botAI) { return new KarathressTidalSurgeTrigger(botAI); }
    static Trigger* karathress_totems(PlayerbotAI* botAI) { return new KarathressTotemsTrigger(botAI); }
    
    // Morogrim Tidewalker
    static Trigger* morogrim_tidal_wave(PlayerbotAI* botAI) { return new MorogrimTidalWaveTrigger(botAI); }
    static Trigger* morogrim_watery_grave(PlayerbotAI* botAI) { return new MorogrimWateryGraveTrigger(botAI); }
    static Trigger* morogrim_murlocs(PlayerbotAI* botAI) { return new MorogrimMurlocsTrigger(botAI); }
    static Trigger* morogrim_globules(PlayerbotAI* botAI) { return new MorogrimGlobulesTrigger(botAI); }
    static Trigger* morogrim_position(PlayerbotAI* botAI) { return new MorogrimPositionTrigger(botAI); }
    
    // Lady Vashj
    static Trigger* vashj_shock_blast(PlayerbotAI* botAI) { return new VashjShockBlastTrigger(botAI); }
    static Trigger* vashj_static_charge(PlayerbotAI* botAI) { return new VashjStaticChargeTrigger(botAI); }
    static Trigger* vashj_entangle(PlayerbotAI* botAI) { return new VashjEntangleTrigger(botAI); }
    static Trigger* vashj_enchanted_elemental(PlayerbotAI* botAI) { return new VashjEnchantedElementalTrigger(botAI); }
    static Trigger* vashj_tainted_elemental(PlayerbotAI* botAI) { return new VashjTaintedElementalTrigger(botAI); }
    static Trigger* vashj_coilfang_elite(PlayerbotAI* botAI) { return new VashjCoilfangEliteTrigger(botAI); }
    static Trigger* vashj_coilfang_strider(PlayerbotAI* botAI) { return new VashjCoilfangStriderTrigger(botAI); }
    static Trigger* vashj_sporebat(PlayerbotAI* botAI) { return new VashjSporebatTrigger(botAI); }
    static Trigger* vashj_position(PlayerbotAI* botAI) { return new VashjPositionTrigger(botAI); }
    static Trigger* vashj_tainted_core(PlayerbotAI* botAI) { return new VashjTaintedCoreTrigger(botAI); }
};

#endif