#ifndef _PLAYERBOT_SERPENTSHRINEACTIONCONTEXT_H
#define _PLAYERBOT_SERPENTSHRINEACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "SerpentshrineActions.h"

class SerpentshrineActionContext : public NamedObjectContext<Action>
{
public:
    SerpentshrineActionContext()
    {
        creators["hydross avoid mark of hydross"] = &SerpentshrineActionContext::hydross_avoid_mark_of_hydross;
        creators["hydross avoid mark of corruption"] = &SerpentshrineActionContext::hydross_avoid_mark_of_corruption;
        creators["hydross water tomb spread"] = &SerpentshrineActionContext::hydross_water_tomb_spread;
        creators["hydross vile sludge spread"] = &SerpentshrineActionContext::hydross_vile_sludge_spread;
        creators["hydross kill adds"] = &SerpentshrineActionContext::hydross_kill_adds;
        creators["hydross position tank"] = &SerpentshrineActionContext::hydross_position_tank;
        creators["hydross transition control"] = &SerpentshrineActionContext::hydross_transition_control;
        
        // The Lurker Below
        creators["lurker spout"] = &SerpentshrineActionContext::lurker_spout;
        creators["lurker whirl avoid"] = &SerpentshrineActionContext::lurker_whirl_avoid;
        creators["lurker geyser spread"] = &SerpentshrineActionContext::lurker_geyser_spread;
        creators["lurker kill adds"] = &SerpentshrineActionContext::lurker_kill_adds;
        creators["lurker position"] = &SerpentshrineActionContext::lurker_position;
        
        // Leotheras the Blind
        creators["leotheras whirlwind"] = &SerpentshrineActionContext::leotheras_whirlwind;
        creators["leotheras chaos blast"] = &SerpentshrineActionContext::leotheras_chaos_blast;
        creators["leotheras inner demon"] = &SerpentshrineActionContext::leotheras_inner_demon;
        creators["leotheras shadow"] = &SerpentshrineActionContext::leotheras_shadow;
        creators["leotheras position"] = &SerpentshrineActionContext::leotheras_position;
        
        // Fathom-Lord Karathress
        creators["karathress cataclysmic bolt"] = &SerpentshrineActionContext::karathress_cataclysmic_bolt;
        creators["karathress sear nova"] = &SerpentshrineActionContext::karathress_sear_nova;
        creators["karathress advisors"] = &SerpentshrineActionContext::karathress_advisors;
        creators["karathress cyclone"] = &SerpentshrineActionContext::karathress_cyclone;
        creators["karathress spread"] = &SerpentshrineActionContext::karathress_spread;
        creators["karathress tidal surge"] = &SerpentshrineActionContext::karathress_tidal_surge;
        creators["karathress totems"] = &SerpentshrineActionContext::karathress_totems;
        
        // Morogrim Tidewalker
        creators["morogrim tidal wave"] = &SerpentshrineActionContext::morogrim_tidal_wave;
        creators["morogrim watery grave"] = &SerpentshrineActionContext::morogrim_watery_grave;
        creators["morogrim murlocs"] = &SerpentshrineActionContext::morogrim_murlocs;
        creators["morogrim offtank murlocs"] = &SerpentshrineActionContext::morogrim_offtank_murlocs;
        creators["morogrim globules"] = &SerpentshrineActionContext::morogrim_globules;
        creators["morogrim position"] = &SerpentshrineActionContext::morogrim_position;
        
        // Lady Vashj
        creators["vashj shock blast"] = &SerpentshrineActionContext::vashj_shock_blast;
        creators["vashj static charge"] = &SerpentshrineActionContext::vashj_static_charge;
        creators["vashj entangle"] = &SerpentshrineActionContext::vashj_entangle;
        creators["vashj enchanted elemental"] = &SerpentshrineActionContext::vashj_enchanted_elemental;
        creators["vashj tainted elemental"] = &SerpentshrineActionContext::vashj_tainted_elemental;
        creators["vashj coilfang elite"] = &SerpentshrineActionContext::vashj_coilfang_elite;
        creators["vashj coilfang strider"] = &SerpentshrineActionContext::vashj_coilfang_strider;
        creators["vashj sporebat"] = &SerpentshrineActionContext::vashj_sporebat;
        creators["vashj position"] = &SerpentshrineActionContext::vashj_position;
        creators["vashj tainted core"] = &SerpentshrineActionContext::vashj_tainted_core;
        creators["vashj main tank elite"] = &SerpentshrineActionContext::vashj_main_tank_elite;
        creators["vashj offtank adds"] = &SerpentshrineActionContext::vashj_offtank_adds;
        creators["vashj forked lightning"] = &SerpentshrineActionContext::vashj_forked_lightning;
        creators["vashj elemental overload"] = &SerpentshrineActionContext::vashj_elemental_overload;
        creators["vashj shield generator"] = &SerpentshrineActionContext::vashj_shield_generator;
        creators["vashj multi shot avoid"] = &SerpentshrineActionContext::vashj_multi_shot_avoid;
        creators["vashj strider fear"] = &SerpentshrineActionContext::vashj_strider_fear;
        creators["vashj quadrant position"] = &SerpentshrineActionContext::vashj_quadrant_position;
    }

private:
    static Action* hydross_avoid_mark_of_hydross(PlayerbotAI* botAI) { return new HydrossAvoidMarkOfHydrossAction(botAI); }
    static Action* hydross_avoid_mark_of_corruption(PlayerbotAI* botAI) { return new HydrossAvoidMarkOfCorruptionAction(botAI); }
    static Action* hydross_water_tomb_spread(PlayerbotAI* botAI) { return new HydrossWaterTombSpreadAction(botAI); }
    static Action* hydross_vile_sludge_spread(PlayerbotAI* botAI) { return new HydrossVileSludgeSpreadAction(botAI); }
    static Action* hydross_kill_adds(PlayerbotAI* botAI) { return new HydrossKillAddsAction(botAI); }
    static Action* hydross_position_tank(PlayerbotAI* botAI) { return new HydrossPositionTankAction(botAI); }
    static Action* hydross_transition_control(PlayerbotAI* botAI) { return new HydrossTransitionControlAction(botAI); }
    
    // The Lurker Below
    static Action* lurker_spout(PlayerbotAI* botAI) { return new LurkerSpoutAction(botAI); }
    static Action* lurker_whirl_avoid(PlayerbotAI* botAI) { return new LurkerWhirlAvoidAction(botAI); }
    static Action* lurker_geyser_spread(PlayerbotAI* botAI) { return new LurkerGeyserSpreadAction(botAI); }
    static Action* lurker_kill_adds(PlayerbotAI* botAI) { return new LurkerKillAddsAction(botAI); }
    static Action* lurker_position(PlayerbotAI* botAI) { return new LurkerPositionAction(botAI); }
    
    // Leotheras the Blind
    static Action* leotheras_whirlwind(PlayerbotAI* botAI) { return new LeotherasWhirlwindAction(botAI); }
    static Action* leotheras_chaos_blast(PlayerbotAI* botAI) { return new LeotherasChaosBlastAction(botAI); }
    static Action* leotheras_inner_demon(PlayerbotAI* botAI) { return new LeotherasInnerDemonAction(botAI); }
    static Action* leotheras_shadow(PlayerbotAI* botAI) { return new LeotherasShadowAction(botAI); }
    static Action* leotheras_position(PlayerbotAI* botAI) { return new LeotherasPositionAction(botAI); }
    
    // Fathom-Lord Karathress
    static Action* karathress_cataclysmic_bolt(PlayerbotAI* botAI) { return new KarathressCataclysmicBoltAction(botAI); }
    static Action* karathress_sear_nova(PlayerbotAI* botAI) { return new KarathressSearNovaAction(botAI); }
    static Action* karathress_advisors(PlayerbotAI* botAI) { return new KarathressAdvisorsAction(botAI); }
    static Action* karathress_cyclone(PlayerbotAI* botAI) { return new KarathressCycloneAction(botAI); }
    static Action* karathress_spread(PlayerbotAI* botAI) { return new KarathressSpreadAction(botAI); }
    static Action* karathress_tidal_surge(PlayerbotAI* botAI) { return new KarathressTidalSurgeAction(botAI); }
    static Action* karathress_totems(PlayerbotAI* botAI) { return new KarathressTotemsAction(botAI); }
    
    // Morogrim Tidewalker
    static Action* morogrim_tidal_wave(PlayerbotAI* botAI) { return new MorogrimTidalWaveAction(botAI); }
    static Action* morogrim_watery_grave(PlayerbotAI* botAI) { return new MorogrimWateryGraveAction(botAI); }
    static Action* morogrim_murlocs(PlayerbotAI* botAI) { return new MorogrimMurlocsAction(botAI); }
    static Action* morogrim_offtank_murlocs(PlayerbotAI* botAI) { return new MorogrimOfftankMurlocsAction(botAI); }
    static Action* morogrim_globules(PlayerbotAI* botAI) { return new MorogrimGlobulesAction(botAI); }
    static Action* morogrim_position(PlayerbotAI* botAI) { return new MorogrimPositionAction(botAI); }
    
    // Lady Vashj
    static Action* vashj_shock_blast(PlayerbotAI* botAI) { return new VashjShockBlastAction(botAI); }
    static Action* vashj_static_charge(PlayerbotAI* botAI) { return new VashjStaticChargeAction(botAI); }
    static Action* vashj_entangle(PlayerbotAI* botAI) { return new VashjEntangleAction(botAI); }
    static Action* vashj_enchanted_elemental(PlayerbotAI* botAI) { return new VashjEnchantedElementalAction(botAI); }
    static Action* vashj_tainted_elemental(PlayerbotAI* botAI) { return new VashjTaintedElementalAction(botAI); }
    static Action* vashj_coilfang_elite(PlayerbotAI* botAI) { return new VashjCoilfangEliteAction(botAI); }
    static Action* vashj_coilfang_strider(PlayerbotAI* botAI) { return new VashjCoilfangStriderAction(botAI); }
    static Action* vashj_sporebat(PlayerbotAI* botAI) { return new VashjSporebatAction(botAI); }
    static Action* vashj_position(PlayerbotAI* botAI) { return new VashjPositionAction(botAI); }
    static Action* vashj_tainted_core(PlayerbotAI* botAI) { return new VashjTaintedCoreAction(botAI); }
    static Action* vashj_main_tank_elite(PlayerbotAI* botAI) { return new VashjMainTankEliteAction(botAI); }
    static Action* vashj_offtank_adds(PlayerbotAI* botAI) { return new VashjOfftankAddsAction(botAI); }
    static Action* vashj_forked_lightning(PlayerbotAI* botAI) { return new VashjForkedLightningAction(botAI); }
    static Action* vashj_elemental_overload(PlayerbotAI* botAI) { return new VashjElementalOverloadAction(botAI); }
    static Action* vashj_shield_generator(PlayerbotAI* botAI) { return new VashjShieldGeneratorAction(botAI); }
    static Action* vashj_multi_shot_avoid(PlayerbotAI* botAI) { return new VashjMultiShotAvoidAction(botAI); }
    static Action* vashj_strider_fear(PlayerbotAI* botAI) { return new VashjStriderFearAction(botAI); }
    static Action* vashj_quadrant_position(PlayerbotAI* botAI) { return new VashjQuadrantPositionAction(botAI); }
};

#endif
