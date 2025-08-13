#ifndef _PLAYERBOT_RAIDZAACTIONCONTEXT_H
#define _PLAYERBOT_RAIDZAACTIONCONTEXT_H

#include "ZulAmanActions.h"

class RaidZaActionContext : public NamedObjectContext<Action>
{
public:
    RaidZaActionContext()
    {
        // Nalorakk
        creators["nalorakk brutal swipe avoid"] = &RaidZaActionContext::nalorakk_brutal_swipe_avoid;
        creators["nalorakk surge position"] = &RaidZaActionContext::nalorakk_surge_position;
        creators["nalorakk bear form position"] = &RaidZaActionContext::nalorakk_bear_form_position;
        
        // Akil'zon
        creators["akilzon electrical storm"] = &RaidZaActionContext::akilzon_electrical_storm;
        creators["akilzon static disruption avoid"] = &RaidZaActionContext::akilzon_static_disruption_avoid;
        creators["akilzon eagle target"] = &RaidZaActionContext::akilzon_eagle_target;
        
        // Jan'alai
        creators["janalai fire bomb avoid"] = &RaidZaActionContext::janalai_fire_bomb_avoid;
        creators["janalai hatcher target"] = &RaidZaActionContext::janalai_hatcher_target;
        creators["janalai hatchling target"] = &RaidZaActionContext::janalai_hatchling_target;
        creators["janalai fire wall avoid"] = &RaidZaActionContext::janalai_fire_wall_avoid;
        
        // Halazzi
        creators["halazzi lynx target"] = &RaidZaActionContext::halazzi_lynx_target;
        creators["halazzi totem target"] = &RaidZaActionContext::halazzi_totem_target;
        creators["halazzi saber lash position"] = &RaidZaActionContext::halazzi_saber_lash_position;
        
        // Hex Lord Malacrass
        creators["hex lord drain power interrupt"] = &RaidZaActionContext::hex_lord_drain_power_interrupt;
        creators["hex lord add target"] = &RaidZaActionContext::hex_lord_add_target;
        creators["hex lord spirit bolley avoid"] = &RaidZaActionContext::hex_lord_spirit_bolley_avoid;
        
        // Zul'jin
        creators["zuljin grievous throw heal"] = &RaidZaActionContext::zuljin_grievous_throw_heal;
        creators["zuljin creeping paralysis avoid"] = &RaidZaActionContext::zuljin_creeping_paralysis_avoid;
        creators["zuljin feather vortex target"] = &RaidZaActionContext::zuljin_feather_vortex_target;
        creators["zuljin cyclone avoid"] = &RaidZaActionContext::zuljin_cyclone_avoid;
        creators["zuljin flame column avoid"] = &RaidZaActionContext::zuljin_flame_column_avoid;
    }

private:
    // Nalorakk
    static Action* nalorakk_brutal_swipe_avoid(PlayerbotAI* ai) { return new NalorakkBrutalSwipeAvoidAction(ai); }
    static Action* nalorakk_surge_position(PlayerbotAI* ai) { return new NalorakkSurgePositionAction(ai); }
    static Action* nalorakk_bear_form_position(PlayerbotAI* ai) { return new NalorakkBearFormPositionAction(ai); }
    
    // Akil'zon
    static Action* akilzon_electrical_storm(PlayerbotAI* ai) { return new AkilzonElectricalStormAction(ai); }
    static Action* akilzon_static_disruption_avoid(PlayerbotAI* ai) { return new AkilzonStaticDisruptionAvoidAction(ai); }
    static Action* akilzon_eagle_target(PlayerbotAI* ai) { return new AkilzonSoaringEagleTargetAction(ai); }
    
    // Jan'alai
    static Action* janalai_fire_bomb_avoid(PlayerbotAI* ai) { return new JanalaiFireBombAvoidAction(ai); }
    static Action* janalai_hatcher_target(PlayerbotAI* ai) { return new JanalaiHatcherTargetAction(ai); }
    static Action* janalai_hatchling_target(PlayerbotAI* ai) { return new JanalaiHatchlingTargetAction(ai); }
    static Action* janalai_fire_wall_avoid(PlayerbotAI* ai) { return new JanalaiFireWallAvoidAction(ai); }
    
    // Halazzi
    static Action* halazzi_lynx_target(PlayerbotAI* ai) { return new HalazziLynxTargetAction(ai); }
    static Action* halazzi_totem_target(PlayerbotAI* ai) { return new HalazziTotemTargetAction(ai); }
    static Action* halazzi_saber_lash_position(PlayerbotAI* ai) { return new HalazziSaberLashPositionAction(ai); }
    
    // Hex Lord Malacrass
    static Action* hex_lord_drain_power_interrupt(PlayerbotAI* ai) { return new HexLordDrainPowerInterruptAction(ai); }
    static Action* hex_lord_add_target(PlayerbotAI* ai) { return new HexLordAddTargetAction(ai); }
    static Action* hex_lord_spirit_bolley_avoid(PlayerbotAI* ai) { return new HexLordSpiritBolleyAvoidAction(ai); }
    
    // Zul'jin
    static Action* zuljin_grievous_throw_heal(PlayerbotAI* ai) { return new ZuljinGrievousThrowHealAction(ai); }
    static Action* zuljin_creeping_paralysis_avoid(PlayerbotAI* ai) { return new ZuljinCreepingParalysisAvoidAction(ai); }
    static Action* zuljin_feather_vortex_target(PlayerbotAI* ai) { return new ZuljinFeatherVortexTargetAction(ai); }
    static Action* zuljin_cyclone_avoid(PlayerbotAI* ai) { return new ZuljinCycloneAvoidAction(ai); }
    static Action* zuljin_flame_column_avoid(PlayerbotAI* ai) { return new ZuljinFlameColumnAvoidAction(ai); }
};

#endif