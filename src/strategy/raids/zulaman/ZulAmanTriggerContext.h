#ifndef _PLAYERBOT_RAIDZATRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDZATRIGGERCONTEXT_H

#include "ZulAmanTriggers.h"

class RaidZaTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZaTriggerContext()
    {
        // Nalorakk
        creators["nalorakk brutal swipe"] = &RaidZaTriggerContext::nalorakk_brutal_swipe;
        creators["nalorakk surge"] = &RaidZaTriggerContext::nalorakk_surge;
        creators["nalorakk bear form"] = &RaidZaTriggerContext::nalorakk_bear_form;
        
        // Akil'zon
        creators["akilzon electrical storm"] = &RaidZaTriggerContext::akilzon_electrical_storm;
        creators["akilzon static disruption"] = &RaidZaTriggerContext::akilzon_static_disruption;
        creators["akilzon soaring eagle"] = &RaidZaTriggerContext::akilzon_soaring_eagle;
        
        // Jan'alai
        creators["janalai fire bomb"] = &RaidZaTriggerContext::janalai_fire_bomb;
        creators["janalai hatcher"] = &RaidZaTriggerContext::janalai_hatcher;
        creators["janalai hatchling"] = &RaidZaTriggerContext::janalai_hatchling;
        creators["janalai fire wall"] = &RaidZaTriggerContext::janalai_fire_wall;
        
        // Halazzi
        creators["halazzi lynx"] = &RaidZaTriggerContext::halazzi_lynx;
        creators["halazzi totem"] = &RaidZaTriggerContext::halazzi_totem;
        creators["halazzi saber lash"] = &RaidZaTriggerContext::halazzi_saber_lash;
        
        // Hex Lord Malacrass
        creators["hex lord drain power"] = &RaidZaTriggerContext::hex_lord_drain_power;
        creators["hex lord add"] = &RaidZaTriggerContext::hex_lord_add;
        creators["hex lord spirit bolley"] = &RaidZaTriggerContext::hex_lord_spirit_bolley;
        
        // Zul'jin
        creators["zuljin grievous throw"] = &RaidZaTriggerContext::zuljin_grievous_throw;
        creators["zuljin creeping paralysis"] = &RaidZaTriggerContext::zuljin_creeping_paralysis;
        creators["zuljin feather vortex"] = &RaidZaTriggerContext::zuljin_feather_vortex;
        creators["zuljin cyclone"] = &RaidZaTriggerContext::zuljin_cyclone;
        creators["zuljin flame column"] = &RaidZaTriggerContext::zuljin_flame_column;
    }

private:
    // Nalorakk
    static Trigger* nalorakk_brutal_swipe(PlayerbotAI* ai) { return new NalorakkBrutalSwipeTrigger(ai); }
    static Trigger* nalorakk_surge(PlayerbotAI* ai) { return new NalorakkSurgeTrigger(ai); }
    static Trigger* nalorakk_bear_form(PlayerbotAI* ai) { return new NalorakkBearFormTrigger(ai); }
    
    // Akil'zon
    static Trigger* akilzon_electrical_storm(PlayerbotAI* ai) { return new AkilzonElectricalStormTrigger(ai); }
    static Trigger* akilzon_static_disruption(PlayerbotAI* ai) { return new AkilzonStaticDisruptionTrigger(ai); }
    static Trigger* akilzon_soaring_eagle(PlayerbotAI* ai) { return new AkilzonSoaringEagleTrigger(ai); }
    
    // Jan'alai
    static Trigger* janalai_fire_bomb(PlayerbotAI* ai) { return new JanalaiFireBombTrigger(ai); }
    static Trigger* janalai_hatcher(PlayerbotAI* ai) { return new JanalaiHatcherTrigger(ai); }
    static Trigger* janalai_hatchling(PlayerbotAI* ai) { return new JanalaiHatchlingTrigger(ai); }
    static Trigger* janalai_fire_wall(PlayerbotAI* ai) { return new JanalaiFireWallTrigger(ai); }
    
    // Halazzi
    static Trigger* halazzi_lynx(PlayerbotAI* ai) { return new HalazziLynxTrigger(ai); }
    static Trigger* halazzi_totem(PlayerbotAI* ai) { return new HalazziTotemTrigger(ai); }
    static Trigger* halazzi_saber_lash(PlayerbotAI* ai) { return new HalazziSaberLashTrigger(ai); }
    
    // Hex Lord Malacrass
    static Trigger* hex_lord_drain_power(PlayerbotAI* ai) { return new HexLordDrainPowerTrigger(ai); }
    static Trigger* hex_lord_add(PlayerbotAI* ai) { return new HexLordAddTrigger(ai); }
    static Trigger* hex_lord_spirit_bolley(PlayerbotAI* ai) { return new HexLordSpiritBolleyTrigger(ai); }
    
    // Zul'jin
    static Trigger* zuljin_grievous_throw(PlayerbotAI* ai) { return new ZuljinGrievousThrowTrigger(ai); }
    static Trigger* zuljin_creeping_paralysis(PlayerbotAI* ai) { return new ZuljinCreepingParalysisTrigger(ai); }
    static Trigger* zuljin_feather_vortex(PlayerbotAI* ai) { return new ZuljinFeatherVortexTrigger(ai); }
    static Trigger* zuljin_cyclone(PlayerbotAI* ai) { return new ZuljinCycloneTrigger(ai); }
    static Trigger* zuljin_flame_column(PlayerbotAI* ai) { return new ZuljinFlameColumnTrigger(ai); }
};

#endif