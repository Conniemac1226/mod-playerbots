#ifndef _PLAYERBOT_SERPENTSHRINEACTIONS_H
#define _PLAYERBOT_SERPENTSHRINEACTIONS_H

#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "MovementActions.h"
#include "Value.h"

constexpr uint32 const NPC_HYDROSS_THE_UNSTABLE = 21216;
constexpr uint32 const NPC_PURE_SPAWN_OF_HYDROSS = 22035;
constexpr uint32 const NPC_TAINTED_SPAWN_OF_HYDROSS = 22036;

const uint32 SPELL_MARK_OF_HYDROSS1 = 38215;
const uint32 SPELL_MARK_OF_HYDROSS2 = 38216;
const uint32 SPELL_MARK_OF_HYDROSS3 = 38217;
const uint32 SPELL_MARK_OF_HYDROSS4 = 38218;
const uint32 SPELL_MARK_OF_HYDROSS5 = 38231;
const uint32 SPELL_MARK_OF_HYDROSS6 = 40584;

const uint32 SPELL_MARK_OF_CORRUPTION1 = 38219;
const uint32 SPELL_MARK_OF_CORRUPTION2 = 38220;
const uint32 SPELL_MARK_OF_CORRUPTION3 = 38221;
const uint32 SPELL_MARK_OF_CORRUPTION4 = 38222;
const uint32 SPELL_MARK_OF_CORRUPTION5 = 38230;
const uint32 SPELL_MARK_OF_CORRUPTION6 = 40583;

const uint32 SPELL_WATER_TOMB = 38235;
const uint32 SPELL_VILE_SLUDGE = 38246;
const uint32 SPELL_HYDROSS_CORRUPTION = 37961;
const uint32 SPELL_BLUE_BEAM = 38015;

// The Lurker Below
constexpr uint32 const NPC_THE_LURKER_BELOW = 21217;
constexpr uint32 const NPC_COILFANG_GUARDIAN = 21873;
constexpr uint32 const NPC_COILFANG_AMBUSHER = 21865;
constexpr uint32 const NPC_COILFANG_FRENZY = 21508;

const uint32 SPELL_LURKER_SPOUT_VISUAL = 37431;
const uint32 SPELL_LURKER_SPOUT_PERIODIC_1 = 37429;
const uint32 SPELL_LURKER_SPOUT_PERIODIC_2 = 37430;
const uint32 SPELL_LURKER_WHIRL = 37660;
const uint32 SPELL_LURKER_GEYSER = 37478;
const uint32 SPELL_LURKER_WATER_BOLT = 37138;
const uint32 SPELL_LURKER_SUBMERGE_VISUAL = 28819;

// Leotheras the Blind
constexpr uint32 const NPC_LEOTHERAS_THE_BLIND = 21215;
constexpr uint32 const NPC_SHADOW_OF_LEOTHERAS = 21875;
constexpr uint32 const NPC_INNER_DEMON = 21857;

const uint32 SPELL_LEOTHERAS_WHIRLWIND = 37640;
const uint32 SPELL_LEOTHERAS_CHAOS_BLAST = 37674;
const uint32 SPELL_LEOTHERAS_INSIDIOUS_WHISPER = 37676;
const uint32 SPELL_LEOTHERAS_METAMORPHOSIS = 37673;
const uint32 SPELL_LEOTHERAS_CONSUMING_MADNESS = 37749;

// Fathom-Lord Karathress
constexpr uint32 const NPC_FATHOM_LORD_KARATHRESS = 21214;
constexpr uint32 const NPC_FATHOM_GUARD_SHARKKIS = 21966;
constexpr uint32 const NPC_FATHOM_GUARD_TIDALVESS = 21965;
constexpr uint32 const NPC_FATHOM_GUARD_CARIBDIS = 21964;
constexpr uint32 const NPC_CYCLONE_KARATHRESS = 22104;
constexpr uint32 const NPC_SPITFIRE_TOTEM = 22091;
constexpr uint32 const NPC_GREATER_EARTHBIND_TOTEM = 22486;
constexpr uint32 const NPC_GREATER_POISON_CLEANSING_TOTEM = 22487;

const uint32 SPELL_KARATHRESS_CATACLYSMIC_BOLT = 38441;
const uint32 SPELL_KARATHRESS_SEAR_NOVA = 38445;
const uint32 SPELL_KARATHRESS_TIDAL_SURGE = 38358;
const uint32 SPELL_KARATHRESS_SUMMON_CYCLONE = 38337;

// Morogrim Tidewalker
constexpr uint32 const NPC_MOROGRIM_TIDEWALKER = 21213;
constexpr uint32 const NPC_TIDEWALKER_LURKER = 21920;
constexpr uint32 const NPC_WATER_GLOBULE = 21913;

// Lady Vashj
constexpr uint32 const NPC_LADY_VASHJ = 21212;
constexpr uint32 const NPC_ENCHANTED_ELEMENTAL = 21958;
constexpr uint32 const NPC_TAINTED_ELEMENTAL = 22009;
constexpr uint32 const NPC_COILFANG_ELITE = 22055;
constexpr uint32 const NPC_COILFANG_STRIDER = 22056;
constexpr uint32 const NPC_TOXIC_SPOREBAT = 22140;
const uint32 SPELL_VASHJ_SHOCK_BLAST = 38509;
const uint32 SPELL_VASHJ_STATIC_CHARGE = 38280;
const uint32 SPELL_VASHJ_ENTANGLE = 38316;
const uint32 SPELL_VASHJ_FORKED_LIGHTNING = 38145;
const uint32 SPELL_VASHJ_MAGIC_BARRIER = 38112;
const uint32 SPELL_VASHJ_TOXIC_SPORES = 38574;
const uint32 SPELL_VASHJ_MULTI_SHOT = 38310;
const uint32 SPELL_VASHJ_SHOOT = 37770;

// Lady Vashj Phase 3 - Items and GameObjects
const uint32 ITEM_TAINTED_CORE = 31088;
const uint32 GO_SHIELD_GENERATOR1 = 185051;
const uint32 GO_SHIELD_GENERATOR2 = 185052;
const uint32 GO_SHIELD_GENERATOR3 = 185053;
const uint32 GO_SHIELD_GENERATOR4 = 185054;

const uint32 SPELL_MOROGRIM_TIDAL_WAVE = 37730;
const uint32 SPELL_MOROGRIM_WATERY_GRAVE_1 = 38023;
const uint32 SPELL_MOROGRIM_WATERY_GRAVE_2 = 38024;
const uint32 SPELL_MOROGRIM_WATERY_GRAVE_3 = 38025;
const uint32 SPELL_MOROGRIM_WATERY_GRAVE_4 = 37850;
const uint32 SPELL_MOROGRIM_EARTHQUAKE = 37764;

class HydrossAvoidMarkOfHydrossAction : public MovementAction
{
public:
    HydrossAvoidMarkOfHydrossAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross avoid mark of hydross") {}
    bool Execute(Event event) override;
};

class HydrossAvoidMarkOfCorruptionAction : public MovementAction
{
public:
    HydrossAvoidMarkOfCorruptionAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross avoid mark of corruption") {}
    bool Execute(Event event) override;
};

class HydrossWaterTombSpreadAction : public MovementAction
{
public:
    HydrossWaterTombSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross water tomb spread") {}
    bool Execute(Event event) override;
};

class HydrossVileSludgeSpreadAction : public MovementAction
{
public:
    HydrossVileSludgeSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross vile sludge spread") {}
    bool Execute(Event event) override;
};

class HydrossKillAddsAction : public AttackAction
{
public:
    HydrossKillAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "hydross kill adds") {}
    bool Execute(Event event) override;
};

class HydrossPositionTankAction : public MovementAction
{
public:
    HydrossPositionTankAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross position tank") {}
    bool Execute(Event event) override;
};

class HydrossTransitionControlAction : public MovementAction
{
public:
    HydrossTransitionControlAction(PlayerbotAI* botAI) : MovementAction(botAI, "hydross transition control") {}
    bool Execute(Event event) override;
};

// The Lurker Below Actions
class LurkerSpoutAction : public MovementAction
{
public:
    LurkerSpoutAction(PlayerbotAI* botAI) : MovementAction(botAI, "lurker spout") {}
    bool Execute(Event event) override;
};

class LurkerWhirlAvoidAction : public MovementAction
{
public:
    LurkerWhirlAvoidAction(PlayerbotAI* botAI) : MovementAction(botAI, "lurker whirl avoid") {}
    bool Execute(Event event) override;
};

class LurkerGeyserSpreadAction : public MovementAction
{
public:
    LurkerGeyserSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "lurker geyser spread") {}
    bool Execute(Event event) override;
};

class LurkerKillAddsAction : public AttackAction
{
public:
    LurkerKillAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "lurker kill adds") {}
    bool Execute(Event event) override;
};

class LurkerPositionAction : public MovementAction
{
public:
    LurkerPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "lurker position") {}
    bool Execute(Event event) override;
};

// Leotheras the Blind Actions
class LeotherasWhirlwindAction : public MovementAction
{
public:
    LeotherasWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, "leotheras whirlwind") {}
    bool Execute(Event event) override;
};

class LeotherasChaosBlastAction : public MovementAction
{
public:
    LeotherasChaosBlastAction(PlayerbotAI* botAI) : MovementAction(botAI, "leotheras chaos blast") {}
    bool Execute(Event event) override;
};

class LeotherasInnerDemonAction : public AttackAction
{
public:
    LeotherasInnerDemonAction(PlayerbotAI* botAI) : AttackAction(botAI, "leotheras inner demon") {}
    bool Execute(Event event) override;
};

class LeotherasShadowAction : public AttackAction
{
public:
    LeotherasShadowAction(PlayerbotAI* botAI) : AttackAction(botAI, "leotheras shadow") {}
    bool Execute(Event event) override;
};

class LeotherasPositionAction : public MovementAction
{
public:
    LeotherasPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "leotheras position") {}
    bool Execute(Event event) override;
};

// Fathom-Lord Karathress Actions
class KarathressCataclysmicBoltAction : public MovementAction
{
public:
    KarathressCataclysmicBoltAction(PlayerbotAI* botAI) : MovementAction(botAI, "karathress cataclysmic bolt") {}
    bool Execute(Event event) override;
};

class KarathressSearNovaAction : public MovementAction
{
public:
    KarathressSearNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "karathress sear nova") {}
    bool Execute(Event event) override;
};

class KarathressAdvisorsAction : public AttackAction
{
public:
    KarathressAdvisorsAction(PlayerbotAI* botAI) : AttackAction(botAI, "karathress advisors") {}
    bool Execute(Event event) override;
};

class KarathressCycloneAction : public MovementAction
{
public:
    KarathressCycloneAction(PlayerbotAI* botAI) : MovementAction(botAI, "karathress cyclone") {}
    bool Execute(Event event) override;
};

class KarathressSpreadAction : public Action
{
public:
    KarathressSpreadAction(PlayerbotAI* botAI) : Action(botAI, "karathress spread") {}
    bool Execute(Event event) override;
};


class KarathressTidalSurgeAction : public MovementAction
{
public:
    KarathressTidalSurgeAction(PlayerbotAI* botAI) : MovementAction(botAI, "karathress tidal surge") {}
    bool Execute(Event event) override;
};

class KarathressTotemsAction : public AttackAction
{
public:
    KarathressTotemsAction(PlayerbotAI* botAI) : AttackAction(botAI, "karathress totems") {}
    bool Execute(Event event) override;
};

// Morogrim Tidewalker Actions
class MorogrimTidalWaveAction : public MovementAction
{
public:
    MorogrimTidalWaveAction(PlayerbotAI* botAI) : MovementAction(botAI, "morogrim tidal wave") {}
    bool Execute(Event event) override;
};

class MorogrimWateryGraveAction : public Action
{
public:
    MorogrimWateryGraveAction(PlayerbotAI* botAI) : Action(botAI, "morogrim watery grave") {}
    bool Execute(Event event) override;
};

class MorogrimMurlocsAction : public AttackAction
{
public:
    MorogrimMurlocsAction(PlayerbotAI* botAI) : AttackAction(botAI, "morogrim murlocs") {}
    bool Execute(Event event) override;
};

class MorogrimOfftankMurlocsAction : public AttackAction
{
public:
    MorogrimOfftankMurlocsAction(PlayerbotAI* botAI) : AttackAction(botAI, "morogrim offtank murlocs") {}
    bool Execute(Event event) override;

private:
    bool HandleOfftankAddManagement(Unit* boss);
    bool IsValidMurlocAdd(Unit* unit);
};

class MorogrimGlobulesAction : public MovementAction
{
public:
    MorogrimGlobulesAction(PlayerbotAI* botAI) : MovementAction(botAI, "morogrim globules") {}
    bool Execute(Event event) override;
};

class MorogrimPositionAction : public MovementAction
{
public:
    MorogrimPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "morogrim position") {}
    bool Execute(Event event) override;
};

// Lady Vashj Actions
class VashjShockBlastAction : public MovementAction
{
public:
    VashjShockBlastAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj shock blast") {}
    bool Execute(Event event) override;
};

class VashjStaticChargeAction : public MovementAction
{
public:
    VashjStaticChargeAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj static charge") {}
    bool Execute(Event event) override;
};

class VashjEntangleAction : public AttackAction
{
public:
    VashjEntangleAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj entangle") {}
    bool Execute(Event event) override;
};

class VashjEnchantedElementalAction : public AttackAction
{
public:
    VashjEnchantedElementalAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj enchanted elemental") {}
    bool Execute(Event event) override;
};

class VashjTaintedElementalAction : public AttackAction
{
public:
    VashjTaintedElementalAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj tainted elemental") {}
    bool Execute(Event event) override;
};

class VashjCoilfangEliteAction : public AttackAction
{
public:
    VashjCoilfangEliteAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj coilfang elite") {}
    bool Execute(Event event) override;
};

class VashjCoilfangStriderAction : public AttackAction
{
public:
    VashjCoilfangStriderAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj coilfang strider") {}
    bool Execute(Event event) override;
};

class VashjSporebatAction : public AttackAction
{
public:
    VashjSporebatAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj sporebat") {}
    bool Execute(Event event) override;
};

class VashjPositionAction : public MovementAction
{
public:
    VashjPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj position") {}
    bool Execute(Event event) override;
};

class VashjTaintedCoreAction : public AttackAction
{
public:
    VashjTaintedCoreAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj tainted core") {}
    bool Execute(Event event) override;
};

class VashjMainTankEliteAction : public AttackAction
{
public:
    VashjMainTankEliteAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj main tank elite") {}
    bool Execute(Event event) override;

private:
    bool HandleMainTankAddManagement(Unit* boss);
};

class VashjOfftankAddsAction : public AttackAction
{
public:
    VashjOfftankAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj offtank adds") {}
    bool Execute(Event event) override;

private:
    bool HandleOfftankAddManagement(Unit* boss);
};

class VashjForkedLightningAction : public MovementAction
{
public:
    VashjForkedLightningAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj forked lightning") {}
    bool Execute(Event event) override;
};

class VashjElementalOverloadAction : public AttackAction
{
public:
    VashjElementalOverloadAction(PlayerbotAI* botAI) : AttackAction(botAI, "vashj elemental overload") {}
    bool Execute(Event event) override;
};

class VashjShieldGeneratorAction : public Action
{
public:
    VashjShieldGeneratorAction(PlayerbotAI* botAI) : Action(botAI, "vashj shield generator") {}
    bool Execute(Event event) override;

private:
    bool HandleShieldGeneratorCoordination();
};

class VashjMultiShotAvoidAction : public MovementAction
{
public:
    VashjMultiShotAvoidAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj multi shot avoid") {}
    bool Execute(Event event) override;
};

class VashjStriderFearAction : public MovementAction
{
public:
    VashjStriderFearAction(PlayerbotAI* botAI) : MovementAction(botAI, "vashj strider fear") {}
    bool Execute(Event event) override;
};

#endif
