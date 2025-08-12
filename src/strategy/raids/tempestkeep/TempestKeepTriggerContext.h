#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERCONTEXT_H

#include "TriggerContext.h"
#include "TempestKeepTriggers.h"

class TempestKeepTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TempestKeepTriggerContext()
    {
        // Al'ar triggers
        creators["alar flame quills"] = &TempestKeepTriggerContext::alar_flame_quills;
        creators["alar dive bomb"] = &TempestKeepTriggerContext::alar_dive_bomb;
        creators["alar flame patch"] = &TempestKeepTriggerContext::alar_flame_patch;
        creators["alar platform"] = &TempestKeepTriggerContext::alar_platform;
        creators["alar adds"] = &TempestKeepTriggerContext::alar_adds;
        
        // Void Reaver triggers
        creators["void reaver pounding"] = &TempestKeepTriggerContext::void_reaver_pounding;
        creators["void reaver arcane orb"] = &TempestKeepTriggerContext::void_reaver_arcane_orb;
        creators["void reaver position"] = &TempestKeepTriggerContext::void_reaver_position;
        
        // Solarian triggers
        creators["solarian wrath"] = &TempestKeepTriggerContext::solarian_wrath;
        creators["solarian blinding light"] = &TempestKeepTriggerContext::solarian_blinding_light;
        creators["solarian portal"] = &TempestKeepTriggerContext::solarian_portal;
        creators["solarian adds"] = &TempestKeepTriggerContext::solarian_adds;
        
        // Kael'thas triggers
        creators["kaelthas advisors"] = &TempestKeepTriggerContext::kaelthas_advisors;
        creators["kaelthas weapons"] = &TempestKeepTriggerContext::kaelthas_weapons;
        creators["kaelthas phoenix"] = &TempestKeepTriggerContext::kaelthas_phoenix;
        creators["kaelthas flamestrike"] = &TempestKeepTriggerContext::kaelthas_flamestrike;
        creators["kaelthas gravity lapse"] = &TempestKeepTriggerContext::kaelthas_gravity_lapse;
        creators["kaelthas pyroblast"] = &TempestKeepTriggerContext::kaelthas_pyroblast;
        creators["kaelthas mind control"] = &TempestKeepTriggerContext::kaelthas_mind_control;
        creators["kaelthas nether vapor"] = &TempestKeepTriggerContext::kaelthas_nether_vapor;
        creators["thaladred fixate"] = &TempestKeepTriggerContext::thaladred_fixate;
        creators["capernian conflagration"] = &TempestKeepTriggerContext::capernian_conflagration;
        creators["telonicus remote toy"] = &TempestKeepTriggerContext::telonicus_remote_toy;
    }

private:
    // Al'ar
    static Trigger* alar_flame_quills(PlayerbotAI* botAI) { return new AlarFlameQuillsTrigger(botAI); }
    static Trigger* alar_dive_bomb(PlayerbotAI* botAI) { return new AlarDiveBombTrigger(botAI); }
    static Trigger* alar_flame_patch(PlayerbotAI* botAI) { return new AlarFlamePatchTrigger(botAI); }
    static Trigger* alar_platform(PlayerbotAI* botAI) { return new AlarPlatformTrigger(botAI); }
    static Trigger* alar_adds(PlayerbotAI* botAI) { return new AlarAddsTrigger(botAI); }
    
    // Void Reaver
    static Trigger* void_reaver_pounding(PlayerbotAI* botAI) { return new VoidReaverPoundingTrigger(botAI); }
    static Trigger* void_reaver_arcane_orb(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbTrigger(botAI); }
    static Trigger* void_reaver_position(PlayerbotAI* botAI) { return new VoidReaverPositionTrigger(botAI); }
    
    // Solarian
    static Trigger* solarian_wrath(PlayerbotAI* botAI) { return new SolarianWrathTrigger(botAI); }
    static Trigger* solarian_blinding_light(PlayerbotAI* botAI) { return new SolarianBlindingLightTrigger(botAI); }
    static Trigger* solarian_portal(PlayerbotAI* botAI) { return new SolarianPortalTrigger(botAI); }
    static Trigger* solarian_adds(PlayerbotAI* botAI) { return new SolarianAddsTrigger(botAI); }
    
    // Kael'thas
    static Trigger* kaelthas_advisors(PlayerbotAI* botAI) { return new KaelthasAdvisorsTrigger(botAI); }
    static Trigger* kaelthas_weapons(PlayerbotAI* botAI) { return new KaelthasWeaponsTrigger(botAI); }
    static Trigger* kaelthas_phoenix(PlayerbotAI* botAI) { return new KaelthasPhoenixTrigger(botAI); }
    static Trigger* kaelthas_flamestrike(PlayerbotAI* botAI) { return new KaelthasFlamestrikeTrigger(botAI); }
    static Trigger* kaelthas_gravity_lapse(PlayerbotAI* botAI) { return new KaelthasGravityLapseTrigger(botAI); }
    static Trigger* kaelthas_pyroblast(PlayerbotAI* botAI) { return new KaelthasPyroblastTrigger(botAI); }
    static Trigger* kaelthas_mind_control(PlayerbotAI* botAI) { return new KaelthasMindControlTrigger(botAI); }
    static Trigger* kaelthas_nether_vapor(PlayerbotAI* botAI) { return new KaelthasNetherVaporTrigger(botAI); }
    static Trigger* thaladred_fixate(PlayerbotAI* botAI) { return new ThaladredFixateTrigger(botAI); }
    static Trigger* capernian_conflagration(PlayerbotAI* botAI) { return new CapernianConflagrationTrigger(botAI); }
    static Trigger* telonicus_remote_toy(PlayerbotAI* botAI) { return new TelonicusRemoteToyTrigger(botAI); }
};

#endif