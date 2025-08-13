#ifndef _PLAYERBOT_RAIDBTACTIONS_H
#define _PLAYERBOT_RAIDBTACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Trigger.h"

class NajentusAvoidImpaledTargetAction : public MovementAction
{
public:
    NajentusAvoidImpaledTargetAction(PlayerbotAI* ai) : MovementAction(ai, "najentus avoid impaled") {}
    bool Execute(Event event) override;
};

class NajentusSpinePickupAction : public MovementAction
{
public:
    NajentusSpinePickupAction(PlayerbotAI* ai) : MovementAction(ai, "najentus spine pickup") {}
    bool Execute(Event event) override;
};

class NajentusThrowSpineAction : public AttackAction
{
public:
    NajentusThrowSpineAction(PlayerbotAI* ai) : AttackAction(ai, "najentus throw spine") {}
    bool Execute(Event event) override;
};

class NajentusTidalBurstPositionAction : public MovementAction
{
public:
    NajentusTidalBurstPositionAction(PlayerbotAI* ai) : MovementAction(ai, "najentus tidal burst position") {}
    bool Execute(Event event) override;
};

// Supremus
class SupremusPhaseCheckAction : public Action
{
public:
    SupremusPhaseCheckAction(PlayerbotAI* ai) : Action(ai, "supremus phase check") {}
    bool Execute(Event event) override;
};

class SupremusKiteAction : public MovementAction
{
public:
    SupremusKiteAction(PlayerbotAI* ai) : MovementAction(ai, "supremus kite") {}
    bool Execute(Event event) override;
};

class SupremusAvoidVolcanoAction : public MovementAction
{
public:
    SupremusAvoidVolcanoAction(PlayerbotAI* ai) : MovementAction(ai, "supremus avoid volcano") {}
    bool Execute(Event event) override;
};

class SupremusAvoidMoltenFlameAction : public MovementAction
{
public:
    SupremusAvoidMoltenFlameAction(PlayerbotAI* ai) : MovementAction(ai, "supremus avoid flame") {}
    bool Execute(Event event) override;
};

class SupremusHatefulStrikePositionAction : public MovementAction
{
public:
    SupremusHatefulStrikePositionAction(PlayerbotAI* ai) : MovementAction(ai, "supremus hateful position") {}
    bool Execute(Event event) override;
};

// Shade of Akama
class ShadeOfAkamaChannelerAction : public AttackAction
{
public:
    ShadeOfAkamaChannelerAction(PlayerbotAI* ai) : AttackAction(ai, "shade channeler target") {}
    bool Execute(Event event) override;
};

class ShadeOfAkamaAddsAction : public AttackAction
{
public:
    ShadeOfAkamaAddsAction(PlayerbotAI* ai) : AttackAction(ai, "shade adds target") {}
    bool Execute(Event event) override;
};

class ShadeOfAkamaProtectAkamaAction : public MovementAction
{
public:
    ShadeOfAkamaProtectAkamaAction(PlayerbotAI* ai) : MovementAction(ai, "shade protect akama") {}
    bool Execute(Event event) override;
};

class ShadeOfAkamaPositionAction : public MovementAction
{
public:
    ShadeOfAkamaPositionAction(PlayerbotAI* ai) : MovementAction(ai, "shade position") {}
    bool Execute(Event event) override;
};

// Teron Gorefiend
class TeronGorefiendShadowOfDeathAction : public MovementAction
{
public:
    TeronGorefiendShadowOfDeathAction(PlayerbotAI* ai) : MovementAction(ai, "teron shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendGhostFormAction : public Action
{
public:
    TeronGorefiendGhostFormAction(PlayerbotAI* ai) : Action(ai, "teron ghost form") {}
    bool Execute(Event event) override;
};

class TeronGorefiendDoomBlossomAvoidAction : public MovementAction
{
public:
    TeronGorefiendDoomBlossomAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "teron avoid doom blossom") {}
    bool Execute(Event event) override;
};

class TeronGorefiendIncinerateSpreadAction : public MovementAction
{
public:
    TeronGorefiendIncinerateSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "teron incinerate spread") {}
    bool Execute(Event event) override;
};

class TeronGorefiendCrushingShadowsSpreadAction : public MovementAction
{
public:
    TeronGorefiendCrushingShadowsSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "teron crushing shadows spread") {}
    bool Execute(Event event) override;
};

// Gurtogg Bloodboil
class GurtoggBloodboilPositionAction : public MovementAction
{
public:
    GurtoggBloodboilPositionAction(PlayerbotAI* ai) : MovementAction(ai, "gurtogg bloodboil position") {}
    bool Execute(Event event) override;
};

class GurtoggFelRageTargetAction : public MovementAction
{
public:
    GurtoggFelRageTargetAction(PlayerbotAI* ai) : MovementAction(ai, "gurtogg fel rage target") {}
    bool Execute(Event event) override;
};

class GurtoggArcingSmashAvoidAction : public MovementAction
{
public:
    GurtoggArcingSmashAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "gurtogg arcing smash avoid") {}
    bool Execute(Event event) override;
};

class GurtoggFelAcidBreathAvoidAction : public MovementAction
{
public:
    GurtoggFelAcidBreathAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "gurtogg fel acid breath avoid") {}
    bool Execute(Event event) override;
};

class GurtoggFelGeyserAvoidAction : public MovementAction
{
public:
    GurtoggFelGeyserAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "gurtogg fel geyser avoid") {}
    bool Execute(Event event) override;
};

class GurtoggAcidicWoundTankSwapAction : public Action
{
public:
    GurtoggAcidicWoundTankSwapAction(PlayerbotAI* ai) : Action(ai, "gurtogg acidic wound swap") {}
    bool Execute(Event event) override;
};

// Reliquary of Souls
class ReliquaryEssenceOfSufferingPositionAction : public MovementAction
{
public:
    ReliquaryEssenceOfSufferingPositionAction(PlayerbotAI* ai) : MovementAction(ai, "reliquary suffering position") {}
    bool Execute(Event event) override;
};

class ReliquaryEssenceOfDesireAction : public MovementAction
{
public:
    ReliquaryEssenceOfDesireAction(PlayerbotAI* ai) : MovementAction(ai, "reliquary desire action") {}
    bool Execute(Event event) override;
};

class ReliquaryEssenceOfAngerSpiteAvoidAction : public MovementAction
{
public:
    ReliquaryEssenceOfAngerSpiteAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "reliquary spite avoid") {}
    bool Execute(Event event) override;
};

class ReliquaryPhaseTransitionAction : public MovementAction
{
public:
    ReliquaryPhaseTransitionAction(PlayerbotAI* ai) : MovementAction(ai, "reliquary phase transition") {}
    bool Execute(Event event) override;
};

class ReliquaryEnslavedSoulAction : public AttackAction
{
public:
    ReliquaryEnslavedSoulAction(PlayerbotAI* ai) : AttackAction(ai, "reliquary enslaved soul") {}
    bool Execute(Event event) override;
};

class ReliquarySoulScreamSpreadAction : public MovementAction
{
public:
    ReliquarySoulScreamSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "reliquary soul scream spread") {}
    bool Execute(Event event) override;
};

// Mother Shahraz
class MotherShahrazSaberLashPositionAction : public MovementAction
{
public:
    MotherShahrazSaberLashPositionAction(PlayerbotAI* ai) : MovementAction(ai, "shahraz saber lash position") {}
    bool Execute(Event event) override;
};

class MotherShahrazFatalAttractionAction : public MovementAction
{
public:
    MotherShahrazFatalAttractionAction(PlayerbotAI* ai) : MovementAction(ai, "shahraz fatal attraction") {}
    bool Execute(Event event) override;
};

class MotherShahrazBeamAvoidAction : public MovementAction
{
public:
    MotherShahrazBeamAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "shahraz beam avoid") {}
    bool Execute(Event event) override;
};

class MotherShahrazPrismaticResistanceAction : public Action
{
public:
    MotherShahrazPrismaticResistanceAction(PlayerbotAI* ai) : Action(ai, "shahraz prismatic resistance") {}
    bool Execute(Event event) override;
};

// Illidari Council
class IllidariCouncilTargetAction : public AttackAction
{
public:
    IllidariCouncilTargetAction(PlayerbotAI* ai) : AttackAction(ai, "council target priority") {}
    bool Execute(Event event) override;
};

class IllidariCouncilSpreadAction : public MovementAction
{
public:
    IllidariCouncilSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "council spread") {}
    bool Execute(Event event) override;
};

class IllidariCouncilInterruptMalandeAction : public Action
{
public:
    IllidariCouncilInterruptMalandeAction(PlayerbotAI* ai) : Action(ai, "council interrupt malande") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAvoidConsecrationAction : public MovementAction
{
public:
    IllidariCouncilAvoidConsecrationAction(PlayerbotAI* ai) : MovementAction(ai, "council avoid consecration") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAvoidBlizzardAction : public MovementAction
{
public:
    IllidariCouncilAvoidBlizzardAction(PlayerbotAI* ai) : MovementAction(ai, "council avoid blizzard") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAvoidFlamestrikeAction : public MovementAction
{
public:
    IllidariCouncilAvoidFlamestrikeAction(PlayerbotAI* ai) : MovementAction(ai, "council avoid flamestrike") {}
    bool Execute(Event event) override;
};

class IllidariCouncilPoisonCleanseAction : public Action
{
public:
    IllidariCouncilPoisonCleanseAction(PlayerbotAI* ai) : Action(ai, "council poison cleanse") {}
    bool Execute(Event event) override;
};

class IllidariCouncilDivineWrathAvoidAction : public MovementAction
{
public:
    IllidariCouncilDivineWrathAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "council divine wrath avoid") {}
    bool Execute(Event event) override;
};

// Illidan Stormrage
class IllidanPhaseCheckAction : public Action
{
public:
    IllidanPhaseCheckAction(PlayerbotAI* ai) : Action(ai, "illidan phase check") {}
    bool Execute(Event event) override;
};

class IllidanFlameCrashAvoidAction : public MovementAction
{
public:
    IllidanFlameCrashAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "illidan flame crash avoid") {}
    bool Execute(Event event) override;
};

class IllidanParasiticShadowfiendAction : public AttackAction
{
public:
    IllidanParasiticShadowfiendAction(PlayerbotAI* ai) : AttackAction(ai, "illidan parasitic shadowfiend") {}
    bool Execute(Event event) override;
};

class IllidanDrawSoulHealAction : public Action
{
public:
    IllidanDrawSoulHealAction(PlayerbotAI* ai) : Action(ai, "illidan draw soul heal") {}
    bool Execute(Event event) override;
};

class IllidanAgonizingFlamesAvoidAction : public MovementAction
{
public:
    IllidanAgonizingFlamesAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "illidan agonizing flames avoid") {}
    bool Execute(Event event) override;
};

class IllidanEyeBeamAvoidAction : public MovementAction
{
public:
    IllidanEyeBeamAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "illidan eye beam avoid") {}
    bool Execute(Event event) override;
};

class IllidanDarkBarrageInterruptAction : public Action
{
public:
    IllidanDarkBarrageInterruptAction(PlayerbotAI* ai) : Action(ai, "illidan dark barrage interrupt") {}
    bool Execute(Event event) override;
};

class IllidanFlameOfAzzinothTankAction : public AttackAction
{
public:
    IllidanFlameOfAzzinothTankAction(PlayerbotAI* ai) : AttackAction(ai, "illidan flame of azzinoth tank") {}
    bool Execute(Event event) override;
};

class IllidanShadowDemonAction : public AttackAction
{
public:
    IllidanShadowDemonAction(PlayerbotAI* ai) : AttackAction(ai, "illidan shadow demon") {}
    bool Execute(Event event) override;
};

class IllidanFlameBurstPositionAction : public MovementAction
{
public:
    IllidanFlameBurstPositionAction(PlayerbotAI* ai) : MovementAction(ai, "illidan flame burst position") {}
    bool Execute(Event event) override;
};

class IllidanCageAvoidAction : public MovementAction
{
public:
    IllidanCageAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "illidan cage avoid") {}
    bool Execute(Event event) override;
};

class IllidanEnrageKiteAction : public MovementAction
{
public:
    IllidanEnrageKiteAction(PlayerbotAI* ai) : MovementAction(ai, "illidan enrage kite") {}
    bool Execute(Event event) override;
};

#endif