#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONCONTEXT_H

#include "ActionContext.h"
#include "TempestKeepActions.h"

class TempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    TempestKeepActionContext()
    {
        // Al'ar actions
        creators["alar flame quills"] = &TempestKeepActionContext::alar_flame_quills;
        creators["alar dive bomb"] = &TempestKeepActionContext::alar_dive_bomb;
        creators["alar flame patch"] = &TempestKeepActionContext::alar_flame_patch;
        creators["alar platform"] = &TempestKeepActionContext::alar_platform;
        creators["alar adds"] = &TempestKeepActionContext::alar_adds;
        
        // Void Reaver actions
        creators["void reaver pounding"] = &TempestKeepActionContext::void_reaver_pounding;
        creators["void reaver arcane orb"] = &TempestKeepActionContext::void_reaver_arcane_orb;
        creators["void reaver position"] = &TempestKeepActionContext::void_reaver_position;
        
        // Solarian actions
        creators["solarian wrath"] = &TempestKeepActionContext::solarian_wrath;
        creators["solarian blinding light"] = &TempestKeepActionContext::solarian_blinding_light;
        creators["solarian portal"] = &TempestKeepActionContext::solarian_portal;
        creators["solarian adds"] = &TempestKeepActionContext::solarian_adds;
        
        // Kael'thas actions
        creators["kaelthas advisors"] = &TempestKeepActionContext::kaelthas_advisors;
        creators["kaelthas weapons"] = &TempestKeepActionContext::kaelthas_weapons;
        creators["kaelthas phoenix"] = &TempestKeepActionContext::kaelthas_phoenix;
        creators["kaelthas flamestrike"] = &TempestKeepActionContext::kaelthas_flamestrike;
        creators["kaelthas gravity lapse"] = &TempestKeepActionContext::kaelthas_gravity_lapse;
        creators["kaelthas pyroblast"] = &TempestKeepActionContext::kaelthas_pyroblast;
        creators["kaelthas mind control"] = &TempestKeepActionContext::kaelthas_mind_control;
        creators["kaelthas nether vapor"] = &TempestKeepActionContext::kaelthas_nether_vapor;
        creators["thaladred fixate"] = &TempestKeepActionContext::thaladred_fixate;
        creators["capernian conflagration"] = &TempestKeepActionContext::capernian_conflagration;
        creators["telonicus remote toy"] = &TempestKeepActionContext::telonicus_remote_toy;
    }

private:
    // Al'ar
    static Action* alar_flame_quills(PlayerbotAI* botAI) { return new AlarFlameQuillsAction(botAI); }
    static Action* alar_dive_bomb(PlayerbotAI* botAI) { return new AlarDiveBombAction(botAI); }
    static Action* alar_flame_patch(PlayerbotAI* botAI) { return new AlarFlamePatchAction(botAI); }
    static Action* alar_platform(PlayerbotAI* botAI) { return new AlarPlatformAction(botAI); }
    static Action* alar_adds(PlayerbotAI* botAI) { return new AlarAddsAction(botAI); }
    
    // Void Reaver
    static Action* void_reaver_pounding(PlayerbotAI* botAI) { return new VoidReaverPoundingAction(botAI); }
    static Action* void_reaver_arcane_orb(PlayerbotAI* botAI) { return new VoidReaverArcaneOrbAction(botAI); }
    static Action* void_reaver_position(PlayerbotAI* botAI) { return new VoidReaverPositionAction(botAI); }
    
    // Solarian
    static Action* solarian_wrath(PlayerbotAI* botAI) { return new SolarianWrathAction(botAI); }
    static Action* solarian_blinding_light(PlayerbotAI* botAI) { return new SolarianBlindingLightAction(botAI); }
    static Action* solarian_portal(PlayerbotAI* botAI) { return new SolarianPortalAction(botAI); }
    static Action* solarian_adds(PlayerbotAI* botAI) { return new SolarianAddsAction(botAI); }
    
    // Kael'thas
    static Action* kaelthas_advisors(PlayerbotAI* botAI) { return new KaelthasAdvisorsAction(botAI); }
    static Action* kaelthas_weapons(PlayerbotAI* botAI) { return new KaelthasWeaponsAction(botAI); }
    static Action* kaelthas_phoenix(PlayerbotAI* botAI) { return new KaelthasPhoenixAction(botAI); }
    static Action* kaelthas_flamestrike(PlayerbotAI* botAI) { return new KaelthsFlamestrikeAction(botAI); }
    static Action* kaelthas_gravity_lapse(PlayerbotAI* botAI) { return new KaelthasGravityLapseAction(botAI); }
    static Action* kaelthas_pyroblast(PlayerbotAI* botAI) { return new KaelthasPyroblastAction(botAI); }
    static Action* kaelthas_mind_control(PlayerbotAI* botAI) { return new KaelthasMindControlAction(botAI); }
    static Action* kaelthas_nether_vapor(PlayerbotAI* botAI) { return new KaelthasNetherVaporAction(botAI); }
    static Action* thaladred_fixate(PlayerbotAI* botAI) { return new ThaladredFixateAction(botAI); }
    static Action* capernian_conflagration(PlayerbotAI* botAI) { return new CapernianConflagrationAction(botAI); }
    static Action* telonicus_remote_toy(PlayerbotAI* botAI) { return new TelonicusRemoteToyAction(botAI); }
};

#endif