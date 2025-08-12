#ifndef _PLAYERBOT_RAIDBTTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDBTTRIGGERCONTEXT_H

#include "BlackTempleTriggers.h"

class RaidBtTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBtTriggerContext()
    {
        creators["najentus impaled nearby"] = &RaidBtTriggerContext::najentus_impaled_nearby;
        creators["najentus spine available"] = &RaidBtTriggerContext::najentus_spine_available;
        creators["najentus tidal shield up"] = &RaidBtTriggerContext::najentus_tidal_shield_up;
        creators["najentus tidal burst soon"] = &RaidBtTriggerContext::najentus_tidal_burst_soon;
        
        // Supremus
        creators["supremus engaged"] = &RaidBtTriggerContext::supremus_engaged;
        creators["supremus kite phase"] = &RaidBtTriggerContext::supremus_kite_phase;
        creators["supremus volcano nearby"] = &RaidBtTriggerContext::supremus_volcano_nearby;
        creators["supremus flame nearby"] = &RaidBtTriggerContext::supremus_flame_nearby;
        creators["supremus tank phase"] = &RaidBtTriggerContext::supremus_tank_phase;
        
        // Shade of Akama
        creators["shade channeler active"] = &RaidBtTriggerContext::shade_channeler_active;
        creators["shade adds active"] = &RaidBtTriggerContext::shade_adds_active;
        creators["shade akama low health"] = &RaidBtTriggerContext::shade_akama_low_health;
        creators["shade phase one"] = &RaidBtTriggerContext::shade_phase_one;
        creators["shade phase two"] = &RaidBtTriggerContext::shade_phase_two;
        
        // Teron Gorefiend
        creators["teron shadow of death"] = &RaidBtTriggerContext::teron_shadow_of_death;
        creators["teron ghost form"] = &RaidBtTriggerContext::teron_ghost_form;
        creators["teron doom blossom nearby"] = &RaidBtTriggerContext::teron_doom_blossom_nearby;
        creators["teron incinerate"] = &RaidBtTriggerContext::teron_incinerate;
        creators["teron crushing shadows"] = &RaidBtTriggerContext::teron_crushing_shadows;
        
        // Gurtogg Bloodboil
        creators["gurtogg bloodboil engaged"] = &RaidBtTriggerContext::gurtogg_bloodboil_engaged;
        creators["gurtogg fel rage target"] = &RaidBtTriggerContext::gurtogg_fel_rage_target;
        creators["gurtogg arcing smash"] = &RaidBtTriggerContext::gurtogg_arcing_smash;
        creators["gurtogg fel acid breath"] = &RaidBtTriggerContext::gurtogg_fel_acid_breath;
        creators["gurtogg fel geyser nearby"] = &RaidBtTriggerContext::gurtogg_fel_geyser_nearby;
        creators["gurtogg acidic wound high"] = &RaidBtTriggerContext::gurtogg_acidic_wound_high;
        
        // Reliquary of Souls
        creators["reliquary phase one"] = &RaidBtTriggerContext::reliquary_phase_one;
        creators["reliquary phase two"] = &RaidBtTriggerContext::reliquary_phase_two;
        creators["reliquary phase three"] = &RaidBtTriggerContext::reliquary_phase_three;
        creators["reliquary phase transition"] = &RaidBtTriggerContext::reliquary_phase_transition;
        creators["reliquary enslaved soul active"] = &RaidBtTriggerContext::reliquary_enslaved_soul_active;
        creators["reliquary soul scream"] = &RaidBtTriggerContext::reliquary_soul_scream;
        creators["reliquary spite"] = &RaidBtTriggerContext::reliquary_spite;
        
        // Mother Shahraz
        creators["shahraz engaged"] = &RaidBtTriggerContext::shahraz_engaged;
        creators["shahraz saber lash tank"] = &RaidBtTriggerContext::shahraz_saber_lash_tank;
        creators["shahraz fatal attraction"] = &RaidBtTriggerContext::shahraz_fatal_attraction;
        creators["shahraz beam target"] = &RaidBtTriggerContext::shahraz_beam_target;
        creators["shahraz prismatic aura"] = &RaidBtTriggerContext::shahraz_prismatic_aura;
    }

private:
    static Trigger* najentus_impaled_nearby(PlayerbotAI* ai) { return new NajentusImpaledNearbyTrigger(ai); }
    static Trigger* najentus_spine_available(PlayerbotAI* ai) { return new NajentusSpineAvailableTrigger(ai); }
    static Trigger* najentus_tidal_shield_up(PlayerbotAI* ai) { return new NajentusTidalShieldUpTrigger(ai); }
    static Trigger* najentus_tidal_burst_soon(PlayerbotAI* ai) { return new NajentusTidalBurstSoonTrigger(ai); }
    
    // Supremus
    static Trigger* supremus_engaged(PlayerbotAI* ai) { return new SupremusEngagedTrigger(ai); }
    static Trigger* supremus_kite_phase(PlayerbotAI* ai) { return new SupremusKitePhaseTrigger(ai); }
    static Trigger* supremus_volcano_nearby(PlayerbotAI* ai) { return new SupremusVolcanoNearbyTrigger(ai); }
    static Trigger* supremus_flame_nearby(PlayerbotAI* ai) { return new SupremusMoltenFlameNearbyTrigger(ai); }
    static Trigger* supremus_tank_phase(PlayerbotAI* ai) { return new SupremusTankPhaseTrigger(ai); }
    
    // Shade of Akama
    static Trigger* shade_channeler_active(PlayerbotAI* ai) { return new ShadeOfAkamaChannelerActiveTrigger(ai); }
    static Trigger* shade_adds_active(PlayerbotAI* ai) { return new ShadeOfAkamaAddsActiveTrigger(ai); }
    static Trigger* shade_akama_low_health(PlayerbotAI* ai) { return new ShadeOfAkamaAkamaLowHealthTrigger(ai); }
    static Trigger* shade_phase_one(PlayerbotAI* ai) { return new ShadeOfAkamaPhaseOneTrigger(ai); }
    static Trigger* shade_phase_two(PlayerbotAI* ai) { return new ShadeOfAkamaPhaseTwoTrigger(ai); }
    
    // Teron Gorefiend
    static Trigger* teron_shadow_of_death(PlayerbotAI* ai) { return new TeronGorefiendShadowOfDeathTrigger(ai); }
    static Trigger* teron_ghost_form(PlayerbotAI* ai) { return new TeronGorefiendGhostFormTrigger(ai); }
    static Trigger* teron_doom_blossom_nearby(PlayerbotAI* ai) { return new TeronGorefiendDoomBlossomNearbyTrigger(ai); }
    static Trigger* teron_incinerate(PlayerbotAI* ai) { return new TeronGorefiendIncinerateTrigger(ai); }
    static Trigger* teron_crushing_shadows(PlayerbotAI* ai) { return new TeronGorefiendCrushingShadowsTrigger(ai); }
    
    // Gurtogg Bloodboil
    static Trigger* gurtogg_bloodboil_engaged(PlayerbotAI* ai) { return new GurtoggBloodboilEngagedTrigger(ai); }
    static Trigger* gurtogg_fel_rage_target(PlayerbotAI* ai) { return new GurtoggFelRageTargetTrigger(ai); }
    static Trigger* gurtogg_arcing_smash(PlayerbotAI* ai) { return new GurtoggArcingSmashTrigger(ai); }
    static Trigger* gurtogg_fel_acid_breath(PlayerbotAI* ai) { return new GurtoggFelAcidBreathTrigger(ai); }
    static Trigger* gurtogg_fel_geyser_nearby(PlayerbotAI* ai) { return new GurtoggFelGeyserNearbyTrigger(ai); }
    static Trigger* gurtogg_acidic_wound_high(PlayerbotAI* ai) { return new GurtoggAcidicWoundHighTrigger(ai); }
    
    // Reliquary of Souls
    static Trigger* reliquary_phase_one(PlayerbotAI* ai) { return new ReliquaryPhaseOneTrigger(ai); }
    static Trigger* reliquary_phase_two(PlayerbotAI* ai) { return new ReliquaryPhaseTwoTrigger(ai); }
    static Trigger* reliquary_phase_three(PlayerbotAI* ai) { return new ReliquaryPhaseThreeTrigger(ai); }
    static Trigger* reliquary_phase_transition(PlayerbotAI* ai) { return new ReliquaryPhaseTransitionTrigger(ai); }
    static Trigger* reliquary_enslaved_soul_active(PlayerbotAI* ai) { return new ReliquaryEnslavedSoulActiveTrigger(ai); }
    static Trigger* reliquary_soul_scream(PlayerbotAI* ai) { return new ReliquarySoulScreamTrigger(ai); }
    static Trigger* reliquary_spite(PlayerbotAI* ai) { return new ReliquarySpiteTrigger(ai); }
    
    // Mother Shahraz
    static Trigger* shahraz_engaged(PlayerbotAI* ai) { return new MotherShahrazEngagedTrigger(ai); }
    static Trigger* shahraz_saber_lash_tank(PlayerbotAI* ai) { return new MotherShahrazSaberLashTankTrigger(ai); }
    static Trigger* shahraz_fatal_attraction(PlayerbotAI* ai) { return new MotherShahrazFatalAttractionTrigger(ai); }
    static Trigger* shahraz_beam_target(PlayerbotAI* ai) { return new MotherShahrazBeamTargetTrigger(ai); }
    static Trigger* shahraz_prismatic_aura(PlayerbotAI* ai) { return new MotherShahrazPrismaticAuraTrigger(ai); }
};

#endif