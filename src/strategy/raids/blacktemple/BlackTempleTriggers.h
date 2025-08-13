#ifndef _PLAYERBOT_RAIDBTTRIGGERS_H
#define _PLAYERBOT_RAIDBTTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

class NajentusImpaledNearbyTrigger : public Trigger
{
public:
    NajentusImpaledNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "najentus impaled nearby") {}
    bool IsActive() override;
};

class NajentusSpineAvailableTrigger : public Trigger
{
public:
    NajentusSpineAvailableTrigger(PlayerbotAI* ai) : Trigger(ai, "najentus spine available") {}
    bool IsActive() override;
};

class NajentusTidalShieldUpTrigger : public Trigger
{
public:
    NajentusTidalShieldUpTrigger(PlayerbotAI* ai) : Trigger(ai, "najentus tidal shield up") {}
    bool IsActive() override;
};

class NajentusTidalBurstSoonTrigger : public Trigger
{
public:
    NajentusTidalBurstSoonTrigger(PlayerbotAI* ai) : Trigger(ai, "najentus tidal burst soon") {}
    bool IsActive() override;
};

// Supremus
class SupremusEngagedTrigger : public Trigger
{
public:
    SupremusEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "supremus engaged") {}
    bool IsActive() override;
};

class SupremusKitePhaseTrigger : public Trigger
{
public:
    SupremusKitePhaseTrigger(PlayerbotAI* ai) : Trigger(ai, "supremus kite phase") {}
    bool IsActive() override;
};

class SupremusVolcanoNearbyTrigger : public Trigger
{
public:
    SupremusVolcanoNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "supremus volcano nearby") {}
    bool IsActive() override;
};

class SupremusMoltenFlameNearbyTrigger : public Trigger
{
public:
    SupremusMoltenFlameNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "supremus flame nearby") {}
    bool IsActive() override;
};

class SupremusTankPhaseTrigger : public Trigger
{
public:
    SupremusTankPhaseTrigger(PlayerbotAI* ai) : Trigger(ai, "supremus tank phase") {}
    bool IsActive() override;
};

// Shade of Akama
class ShadeOfAkamaChannelerActiveTrigger : public Trigger
{
public:
    ShadeOfAkamaChannelerActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "shade channeler active") {}
    bool IsActive() override;
};

class ShadeOfAkamaAddsActiveTrigger : public Trigger
{
public:
    ShadeOfAkamaAddsActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "shade adds active") {}
    bool IsActive() override;
};

class ShadeOfAkamaAkamaLowHealthTrigger : public Trigger
{
public:
    ShadeOfAkamaAkamaLowHealthTrigger(PlayerbotAI* ai) : Trigger(ai, "shade akama low health") {}
    bool IsActive() override;
};

class ShadeOfAkamaPhaseOneTrigger : public Trigger
{
public:
    ShadeOfAkamaPhaseOneTrigger(PlayerbotAI* ai) : Trigger(ai, "shade phase one") {}
    bool IsActive() override;
};

class ShadeOfAkamaPhaseTwoTrigger : public Trigger
{
public:
    ShadeOfAkamaPhaseTwoTrigger(PlayerbotAI* ai) : Trigger(ai, "shade phase two") {}
    bool IsActive() override;
};

// Teron Gorefiend
class TeronGorefiendShadowOfDeathTrigger : public Trigger
{
public:
    TeronGorefiendShadowOfDeathTrigger(PlayerbotAI* ai) : Trigger(ai, "teron shadow of death") {}
    bool IsActive() override;
};

class TeronGorefiendGhostFormTrigger : public Trigger
{
public:
    TeronGorefiendGhostFormTrigger(PlayerbotAI* ai) : Trigger(ai, "teron ghost form") {}
    bool IsActive() override;
};

class TeronGorefiendDoomBlossomNearbyTrigger : public Trigger
{
public:
    TeronGorefiendDoomBlossomNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "teron doom blossom nearby") {}
    bool IsActive() override;
};

class TeronGorefiendIncinerateTrigger : public Trigger
{
public:
    TeronGorefiendIncinerateTrigger(PlayerbotAI* ai) : Trigger(ai, "teron incinerate") {}
    bool IsActive() override;
};

class TeronGorefiendCrushingShadowsTrigger : public Trigger
{
public:
    TeronGorefiendCrushingShadowsTrigger(PlayerbotAI* ai) : Trigger(ai, "teron crushing shadows") {}
    bool IsActive() override;
};

// Gurtogg Bloodboil
class GurtoggBloodboilEngagedTrigger : public Trigger
{
public:
    GurtoggBloodboilEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg bloodboil engaged") {}
    bool IsActive() override;
};

class GurtoggFelRageTargetTrigger : public Trigger
{
public:
    GurtoggFelRageTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg fel rage target") {}
    bool IsActive() override;
};

class GurtoggArcingSmashTrigger : public Trigger
{
public:
    GurtoggArcingSmashTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg arcing smash") {}
    bool IsActive() override;
};

class GurtoggFelAcidBreathTrigger : public Trigger
{
public:
    GurtoggFelAcidBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg fel acid breath") {}
    bool IsActive() override;
};

class GurtoggFelGeyserNearbyTrigger : public Trigger
{
public:
    GurtoggFelGeyserNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg fel geyser nearby") {}
    bool IsActive() override;
};

class GurtoggAcidicWoundHighTrigger : public Trigger
{
public:
    GurtoggAcidicWoundHighTrigger(PlayerbotAI* ai) : Trigger(ai, "gurtogg acidic wound high") {}
    bool IsActive() override;
};

// Reliquary of Souls
class ReliquaryPhaseOneTrigger : public Trigger
{
public:
    ReliquaryPhaseOneTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary phase one") {}
    bool IsActive() override;
};

class ReliquaryPhaseTwoTrigger : public Trigger
{
public:
    ReliquaryPhaseTwoTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary phase two") {}
    bool IsActive() override;
};

class ReliquaryPhaseThreeTrigger : public Trigger
{
public:
    ReliquaryPhaseThreeTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary phase three") {}
    bool IsActive() override;
};

class ReliquaryPhaseTransitionTrigger : public Trigger
{
public:
    ReliquaryPhaseTransitionTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary phase transition") {}
    bool IsActive() override;
};

class ReliquaryEnslavedSoulActiveTrigger : public Trigger
{
public:
    ReliquaryEnslavedSoulActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary enslaved soul active") {}
    bool IsActive() override;
};

class ReliquarySoulScreamTrigger : public Trigger
{
public:
    ReliquarySoulScreamTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary soul scream") {}
    bool IsActive() override;
};

class ReliquarySpiteTrigger : public Trigger
{
public:
    ReliquarySpiteTrigger(PlayerbotAI* ai) : Trigger(ai, "reliquary spite") {}
    bool IsActive() override;
};

// Mother Shahraz
class MotherShahrazEngagedTrigger : public Trigger
{
public:
    MotherShahrazEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "shahraz engaged") {}
    bool IsActive() override;
};

class MotherShahrazSaberLashTankTrigger : public Trigger
{
public:
    MotherShahrazSaberLashTankTrigger(PlayerbotAI* ai) : Trigger(ai, "shahraz saber lash tank") {}
    bool IsActive() override;
};

class MotherShahrazFatalAttractionTrigger : public Trigger
{
public:
    MotherShahrazFatalAttractionTrigger(PlayerbotAI* ai) : Trigger(ai, "shahraz fatal attraction") {}
    bool IsActive() override;
};

class MotherShahrazBeamTargetTrigger : public Trigger
{
public:
    MotherShahrazBeamTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "shahraz beam target") {}
    bool IsActive() override;
};

class MotherShahrazPrismaticAuraTrigger : public Trigger
{
public:
    MotherShahrazPrismaticAuraTrigger(PlayerbotAI* ai) : Trigger(ai, "shahraz prismatic aura") {}
    bool IsActive() override;
};

// Illidari Council
class IllidariCouncilEngagedTrigger : public Trigger
{
public:
    IllidariCouncilEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "council engaged") {}
    bool IsActive() override;
};

class IllidariCouncilTargetPriorityTrigger : public Trigger
{
public:
    IllidariCouncilTargetPriorityTrigger(PlayerbotAI* ai) : Trigger(ai, "council target priority") {}
    bool IsActive() override;
};

class IllidariCouncilSpreadTrigger : public Trigger
{
public:
    IllidariCouncilSpreadTrigger(PlayerbotAI* ai) : Trigger(ai, "council spread needed") {}
    bool IsActive() override;
};

class IllidariCouncilInterruptMalandeTrigger : public Trigger
{
public:
    IllidariCouncilInterruptMalandeTrigger(PlayerbotAI* ai) : Trigger(ai, "council interrupt malande") {}
    bool IsActive() override;
};

class IllidariCouncilConsecrationTrigger : public Trigger
{
public:
    IllidariCouncilConsecrationTrigger(PlayerbotAI* ai) : Trigger(ai, "council consecration") {}
    bool IsActive() override;
};

class IllidariCouncilBlizzardTrigger : public Trigger
{
public:
    IllidariCouncilBlizzardTrigger(PlayerbotAI* ai) : Trigger(ai, "council blizzard") {}
    bool IsActive() override;
};

class IllidariCouncilFlamestrikeTrigger : public Trigger
{
public:
    IllidariCouncilFlamestrikeTrigger(PlayerbotAI* ai) : Trigger(ai, "council flamestrike") {}
    bool IsActive() override;
};

class IllidariCouncilPoisonTrigger : public Trigger
{
public:
    IllidariCouncilPoisonTrigger(PlayerbotAI* ai) : Trigger(ai, "council poison") {}
    bool IsActive() override;
};

class IllidariCouncilDivineWrathTrigger : public Trigger
{
public:
    IllidariCouncilDivineWrathTrigger(PlayerbotAI* ai) : Trigger(ai, "council divine wrath") {}
    bool IsActive() override;
};

// Illidan Stormrage
class IllidanEngagedTrigger : public Trigger
{
public:
    IllidanEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan engaged") {}
    bool IsActive() override;
};

class IllidanFlameCrashTrigger : public Trigger
{
public:
    IllidanFlameCrashTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan flame crash") {}
    bool IsActive() override;
};

class IllidanParasiticShadowfiendTrigger : public Trigger
{
public:
    IllidanParasiticShadowfiendTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan parasitic shadowfiend") {}
    bool IsActive() override;
};

class IllidanDrawSoulTrigger : public Trigger
{
public:
    IllidanDrawSoulTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan draw soul") {}
    bool IsActive() override;
};

class IllidanAgonizingFlamesTrigger : public Trigger
{
public:
    IllidanAgonizingFlamesTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan agonizing flames") {}
    bool IsActive() override;
};

class IllidanEyeBeamTrigger : public Trigger
{
public:
    IllidanEyeBeamTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan eye beam") {}
    bool IsActive() override;
};

class IllidanDarkBarrageTrigger : public Trigger
{
public:
    IllidanDarkBarrageTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan dark barrage") {}
    bool IsActive() override;
};

class IllidanFlameOfAzzinothTrigger : public Trigger
{
public:
    IllidanFlameOfAzzinothTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan flame of azzinoth") {}
    bool IsActive() override;
};

class IllidanShadowDemonTrigger : public Trigger
{
public:
    IllidanShadowDemonTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan shadow demon") {}
    bool IsActive() override;
};

class IllidanDemonFormTrigger : public Trigger
{
public:
    IllidanDemonFormTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan demon form") {}
    bool IsActive() override;
};

class IllidanCageTrapTrigger : public Trigger
{
public:
    IllidanCageTrapTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan cage trap") {}
    bool IsActive() override;
};

class IllidanEnrageTrigger : public Trigger
{
public:
    IllidanEnrageTrigger(PlayerbotAI* ai) : Trigger(ai, "illidan enrage") {}
    bool IsActive() override;
};

#endif