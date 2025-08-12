#ifndef _PLAYERBOT_RAIDBTACTIONCONTEXT_H
#define _PLAYERBOT_RAIDBTACTIONCONTEXT_H

#include "BlackTempleActions.h"

class RaidBtActionContext : public NamedObjectContext<Action>
{
public:
    RaidBtActionContext()
    {
        creators["najentus avoid impaled"] = &RaidBtActionContext::najentus_avoid_impaled;
        creators["najentus spine pickup"] = &RaidBtActionContext::najentus_spine_pickup;
        creators["najentus throw spine"] = &RaidBtActionContext::najentus_throw_spine;
        creators["najentus tidal burst position"] = &RaidBtActionContext::najentus_tidal_burst_position;
        
        // Supremus
        creators["supremus phase check"] = &RaidBtActionContext::supremus_phase_check;
        creators["supremus kite"] = &RaidBtActionContext::supremus_kite;
        creators["supremus avoid volcano"] = &RaidBtActionContext::supremus_avoid_volcano;
        creators["supremus avoid flame"] = &RaidBtActionContext::supremus_avoid_flame;
        creators["supremus hateful position"] = &RaidBtActionContext::supremus_hateful_position;
        
        // Shade of Akama
        creators["shade channeler target"] = &RaidBtActionContext::shade_channeler_target;
        creators["shade adds target"] = &RaidBtActionContext::shade_adds_target;
        creators["shade protect akama"] = &RaidBtActionContext::shade_protect_akama;
        creators["shade position"] = &RaidBtActionContext::shade_position;
        
        // Teron Gorefiend
        creators["teron shadow of death"] = &RaidBtActionContext::teron_shadow_of_death;
        creators["teron ghost form"] = &RaidBtActionContext::teron_ghost_form;
        creators["teron avoid doom blossom"] = &RaidBtActionContext::teron_avoid_doom_blossom;
        creators["teron incinerate spread"] = &RaidBtActionContext::teron_incinerate_spread;
        creators["teron crushing shadows spread"] = &RaidBtActionContext::teron_crushing_shadows_spread;
        
        // Gurtogg Bloodboil
        creators["gurtogg bloodboil position"] = &RaidBtActionContext::gurtogg_bloodboil_position;
        creators["gurtogg fel rage target"] = &RaidBtActionContext::gurtogg_fel_rage_target;
        creators["gurtogg arcing smash avoid"] = &RaidBtActionContext::gurtogg_arcing_smash_avoid;
        creators["gurtogg fel acid breath avoid"] = &RaidBtActionContext::gurtogg_fel_acid_breath_avoid;
        creators["gurtogg fel geyser avoid"] = &RaidBtActionContext::gurtogg_fel_geyser_avoid;
        creators["gurtogg acidic wound swap"] = &RaidBtActionContext::gurtogg_acidic_wound_swap;
        
        // Reliquary of Souls
        creators["reliquary suffering position"] = &RaidBtActionContext::reliquary_suffering_position;
        creators["reliquary desire action"] = &RaidBtActionContext::reliquary_desire_action;
        creators["reliquary spite avoid"] = &RaidBtActionContext::reliquary_spite_avoid;
        creators["reliquary phase transition"] = &RaidBtActionContext::reliquary_phase_transition;
        creators["reliquary enslaved soul"] = &RaidBtActionContext::reliquary_enslaved_soul;
        creators["reliquary soul scream spread"] = &RaidBtActionContext::reliquary_soul_scream_spread;
        
        // Mother Shahraz
        creators["shahraz saber lash position"] = &RaidBtActionContext::shahraz_saber_lash_position;
        creators["shahraz fatal attraction"] = &RaidBtActionContext::shahraz_fatal_attraction;
        creators["shahraz beam avoid"] = &RaidBtActionContext::shahraz_beam_avoid;
        creators["shahraz prismatic resistance"] = &RaidBtActionContext::shahraz_prismatic_resistance;
    }

private:
    static Action* najentus_avoid_impaled(PlayerbotAI* ai) { return new NajentusAvoidImpaledTargetAction(ai); }
    static Action* najentus_spine_pickup(PlayerbotAI* ai) { return new NajentusSpinePickupAction(ai); }
    static Action* najentus_throw_spine(PlayerbotAI* ai) { return new NajentusThrowSpineAction(ai); }
    static Action* najentus_tidal_burst_position(PlayerbotAI* ai) { return new NajentusTidalBurstPositionAction(ai); }
    
    // Supremus
    static Action* supremus_phase_check(PlayerbotAI* ai) { return new SupremusPhaseCheckAction(ai); }
    static Action* supremus_kite(PlayerbotAI* ai) { return new SupremusKiteAction(ai); }
    static Action* supremus_avoid_volcano(PlayerbotAI* ai) { return new SupremusAvoidVolcanoAction(ai); }
    static Action* supremus_avoid_flame(PlayerbotAI* ai) { return new SupremusAvoidMoltenFlameAction(ai); }
    static Action* supremus_hateful_position(PlayerbotAI* ai) { return new SupremusHatefulStrikePositionAction(ai); }
    
    // Shade of Akama
    static Action* shade_channeler_target(PlayerbotAI* ai) { return new ShadeOfAkamaChannelerAction(ai); }
    static Action* shade_adds_target(PlayerbotAI* ai) { return new ShadeOfAkamaAddsAction(ai); }
    static Action* shade_protect_akama(PlayerbotAI* ai) { return new ShadeOfAkamaProtectAkamaAction(ai); }
    static Action* shade_position(PlayerbotAI* ai) { return new ShadeOfAkamaPositionAction(ai); }
    
    // Teron Gorefiend
    static Action* teron_shadow_of_death(PlayerbotAI* ai) { return new TeronGorefiendShadowOfDeathAction(ai); }
    static Action* teron_ghost_form(PlayerbotAI* ai) { return new TeronGorefiendGhostFormAction(ai); }
    static Action* teron_avoid_doom_blossom(PlayerbotAI* ai) { return new TeronGorefiendDoomBlossomAvoidAction(ai); }
    static Action* teron_incinerate_spread(PlayerbotAI* ai) { return new TeronGorefiendIncinerateSpreadAction(ai); }
    static Action* teron_crushing_shadows_spread(PlayerbotAI* ai) { return new TeronGorefiendCrushingShadowsSpreadAction(ai); }
    
    // Gurtogg Bloodboil
    static Action* gurtogg_bloodboil_position(PlayerbotAI* ai) { return new GurtoggBloodboilPositionAction(ai); }
    static Action* gurtogg_fel_rage_target(PlayerbotAI* ai) { return new GurtoggFelRageTargetAction(ai); }
    static Action* gurtogg_arcing_smash_avoid(PlayerbotAI* ai) { return new GurtoggArcingSmashAvoidAction(ai); }
    static Action* gurtogg_fel_acid_breath_avoid(PlayerbotAI* ai) { return new GurtoggFelAcidBreathAvoidAction(ai); }
    static Action* gurtogg_fel_geyser_avoid(PlayerbotAI* ai) { return new GurtoggFelGeyserAvoidAction(ai); }
    static Action* gurtogg_acidic_wound_swap(PlayerbotAI* ai) { return new GurtoggAcidicWoundTankSwapAction(ai); }
    
    // Reliquary of Souls
    static Action* reliquary_suffering_position(PlayerbotAI* ai) { return new ReliquaryEssenceOfSufferingPositionAction(ai); }
    static Action* reliquary_desire_action(PlayerbotAI* ai) { return new ReliquaryEssenceOfDesireAction(ai); }
    static Action* reliquary_spite_avoid(PlayerbotAI* ai) { return new ReliquaryEssenceOfAngerSpiteAvoidAction(ai); }
    static Action* reliquary_phase_transition(PlayerbotAI* ai) { return new ReliquaryPhaseTransitionAction(ai); }
    static Action* reliquary_enslaved_soul(PlayerbotAI* ai) { return new ReliquaryEnslavedSoulAction(ai); }
    static Action* reliquary_soul_scream_spread(PlayerbotAI* ai) { return new ReliquarySoulScreamSpreadAction(ai); }
    
    // Mother Shahraz
    static Action* shahraz_saber_lash_position(PlayerbotAI* ai) { return new MotherShahrazSaberLashPositionAction(ai); }
    static Action* shahraz_fatal_attraction(PlayerbotAI* ai) { return new MotherShahrazFatalAttractionAction(ai); }
    static Action* shahraz_beam_avoid(PlayerbotAI* ai) { return new MotherShahrazBeamAvoidAction(ai); }
    static Action* shahraz_prismatic_resistance(PlayerbotAI* ai) { return new MotherShahrazPrismaticResistanceAction(ai); }
};

#endif