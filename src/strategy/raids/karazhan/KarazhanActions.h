#ifndef _PLAYERBOT_KARAZHANACTIONS_H
#define _PLAYERBOT_KARAZHANACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

enum KarazhanNPCs
{
    // Attumen
    NPC_MIDNIGHT                = 16151,
    NPC_ATTUMEN_UNMOUNTED      = 15550,
    NPC_ATTUMEN_MOUNTED        = 16152,
    
    // Moroes
    NPC_MOROES                 = 15687,
    NPC_BARONESS_DOROTHEA      = 19875,
    NPC_LADY_CATRIONA          = 19872,
    NPC_LADY_KEIRA             = 19873,
    NPC_LORD_ROBIN             = 19874,
    NPC_LORD_CRISPIN           = 19871,
    NPC_BARON_RAFE             = 19876,
    
    // Maiden of Virtue
    NPC_MAIDEN_OF_VIRTUE       = 16457,
    
    // Opera Event
    NPC_DOROTHEE               = 17535,
    NPC_ROAR                   = 17546,
    NPC_STRAWMAN               = 17543,
    NPC_TINHEAD                = 17547,
    NPC_TITO                   = 17548,
    NPC_CRONE                  = 18168,
    NPC_ROMULO                 = 17533,
    NPC_JULIANNE               = 17534,
    NPC_BIG_BAD_WOLF           = 17521,
    
    // Curator
    NPC_CURATOR                = 15691,
    NPC_ASTRAL_FLARE           = 17096,
    
    // Shade of Aran
    NPC_SHADE_OF_ARAN          = 16524,
    NPC_WATER_ELEMENTAL        = 17167,
    
    // Terestian Illhoof
    NPC_TERESTIAN_ILLHOOF      = 15688,
    NPC_KILTREK                = 17229,
    NPC_DEMON_CHAINS           = 17248,
    NPC_FIENDISH_IMP           = 17267,
    
    // Netherspite
    NPC_NETHERSPITE            = 15689,
    
    // Chess Event
    NPC_ECHO_OF_MEDIVH         = 16816,
    NPC_CHESS_KING_LLANE       = 21684,
    NPC_WARCHIEF_BLACKHAND     = 21752,
    NPC_HUMAN_FOOTMAN          = 17211,
    NPC_ORC_GRUNT              = 17469,
    NPC_HUMAN_CHARGER          = 21664,
    NPC_ORC_WOLF               = 21748,
    NPC_HUMAN_CONJURER         = 21683,
    NPC_ORC_WARLOCK            = 21750,
    NPC_HUMAN_CLERIC           = 21682,
    NPC_ORC_NECROLYTE          = 21747,
    NPC_CONJURED_WATER_ELEMENTAL = 21160,
    
    // Prince Malchezaar
    NPC_PRINCE_MALCHEZAAR      = 15690,
    NPC_NETHERSPITE_INFERNAL   = 17646,
    
    // Nightbane
    NPC_NIGHTBANE              = 17225,
    NPC_RESTLESS_SKELETON      = 17261
};

enum KarazhanSpells
{
    // Attumen
    SPELL_SHADOWCLEAVE         = 29832,
    SPELL_INTANGIBLE_PRESENCE  = 29833,
    SPELL_KNOCKDOWN            = 29711,
    SPELL_CHARGE               = 29847,
    
    // Moroes
    SPELL_VANISH               = 29448,
    SPELL_GARROTE              = 37066,
    SPELL_BLIND                = 34694,
    SPELL_GOUGE                = 29425,
    SPELL_MOROES_ENRAGE        = 37023,
    
    // Maiden of Virtue
    SPELL_REPENTANCE           = 29511,
    SPELL_HOLY_FIRE            = 29522,
    SPELL_HOLY_WRATH           = 32445,
    SPELL_HOLY_GROUND          = 29523,
    
    // Curator
    SPELL_HATEFUL_BOLT         = 30383,
    SPELL_CURATOR_EVOCATION    = 30254,
    
    // Shade of Aran
    SPELL_ARCANE_MISSILES      = 29955,
    SPELL_CIRCULAR_BLIZZARD    = 29952,
    SPELL_FLAME_WREATH         = 29946,
    SPELL_DRAGONS_BREATH       = 29964,
    SPELL_MASS_POLYMORPH       = 29963,
    SPELL_ARAN_ARCANE_EXPLOSION = 29973,
    
    // Terestian Illhoof
    SPELL_ILLHOOF_SHADOW_BOLT  = 30055,
    SPELL_SACRIFICE            = 30115,
    SPELL_SUMMON_DEMONCHAINS   = 30120,
    SPELL_SUMMON_FIENDISHIMP   = 30184,
    
    // Netherspite
    SPELL_NETHERBURN           = 30522,
    SPELL_VOID_ZONE            = 37063,
    SPELL_NETHERBREATH         = 38523,
    
    // Prince Malchezaar
    SPELL_SHADOW_WORD_PAIN     = 30854,
    SPELL_SHADOW_NOVA          = 30852,
    SPELL_THRASH               = 30858,
    SPELL_SUNDER_ARMOR         = 30901,
    SPELL_AMPLIFY_DAMAGE       = 39095,
    SPELL_INFERNAL_RELAY       = 30834,
    SPELL_HELLFIRE             = 30859,
    
    // Nightbane
    SPELL_NIGHTBANE_ROAR       = 39427,
    SPELL_CHARRED_EARTH        = 30129,
    SPELL_DISTRACTING_ASH      = 30130,
    SPELL_SMOLDERING_BREATH    = 30210,
    SPELL_TAIL_SWEEP           = 25653,
    SPELL_RAIN_OF_BONES        = 37098,
    SPELL_SMOKING_BLAST        = 37057,
    SPELL_FIREBALL_BARRAGE     = 30282,
    
    // Chess Event
    SPELL_GAME_IN_SESSION      = 39331
};

// Attumen the Huntsman
class AttumenAvoidChargeAction : public MovementAction
{
public:
    AttumenAvoidChargeAction(PlayerbotAI* ai) : MovementAction(ai, "attumen avoid charge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttumenPositionAction : public MovementAction
{
public:
    AttumenPositionAction(PlayerbotAI* ai) : MovementAction(ai, "attumen position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Moroes
class MoroesFocusAddsAction : public AttackAction
{
public:
    MoroesFocusAddsAction(PlayerbotAI* ai) : AttackAction(ai, "moroes focus adds") {}
    bool Execute(Event event) override;
};

class MoroesPositionAction : public MovementAction
{
public:
    MoroesPositionAction(PlayerbotAI* ai) : MovementAction(ai, "moroes position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoroesCrowdControlAction : public Action
{
public:
    MoroesCrowdControlAction(PlayerbotAI* ai) : Action(ai, "moroes crowd control") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoroesTankSwapAction : public Action
{
public:
    MoroesTankSwapAction(PlayerbotAI* ai) : Action(ai, "moroes tank swap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MoroesAttackAction : public Action
{
public:
    MoroesAttackAction(PlayerbotAI* ai) : Action(ai, "moroes attack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Maiden of Virtue
class MaidenRepentanceAction : public MovementAction
{
public:
    MaidenRepentanceAction(PlayerbotAI* ai) : MovementAction(ai, "maiden repentance") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaidenHolyGroundAction : public MovementAction
{
public:
    MaidenHolyGroundAction(PlayerbotAI* ai) : MovementAction(ai, "maiden holy ground") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Opera Event
class OperaPositionAction : public MovementAction
{
public:
    OperaPositionAction(PlayerbotAI* ai) : MovementAction(ai, "opera position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class OperaFocusTargetAction : public AttackAction
{
public:
    OperaFocusTargetAction(PlayerbotAI* ai) : AttackAction(ai, "opera focus target") {}
    bool Execute(Event event) override;
};

// Curator
class CuratorFlareAction : public AttackAction
{
public:
    CuratorFlareAction(PlayerbotAI* ai) : AttackAction(ai, "curator flare") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class CuratorEvocationAction : public Action
{
public:
    CuratorEvocationAction(PlayerbotAI* ai) : Action(ai, "curator evocation") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Shade of Aran
class AranFlameWreathAction : public MovementAction
{
public:
    AranFlameWreathAction(PlayerbotAI* ai) : MovementAction(ai, "aran flame wreath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AranBlizzardAction : public MovementAction
{
public:
    AranBlizzardAction(PlayerbotAI* ai) : MovementAction(ai, "aran blizzard") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AranDragonsBreathAction : public MovementAction
{
public:
    AranDragonsBreathAction(PlayerbotAI* ai) : MovementAction(ai, "aran dragons breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Terestian Illhoof
class IllhoofDemonChainsAction : public AttackAction
{
public:
    IllhoofDemonChainsAction(PlayerbotAI* ai) : AttackAction(ai, "illhoof demon chains") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IllhoofImpsAction : public AttackAction
{
public:
    IllhoofImpsAction(PlayerbotAI* ai) : AttackAction(ai, "illhoof imps") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Netherspite
class NetherspiteBeamAction : public MovementAction
{
public:
    NetherspiteBeamAction(PlayerbotAI* ai) : MovementAction(ai, "netherspite beam") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NetherspiteVoidZoneAction : public MovementAction
{
public:
    NetherspiteVoidZoneAction(PlayerbotAI* ai) : MovementAction(ai, "netherspite void zone") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Chess Event
class ChessEventMoveAction : public Action
{
public:
    ChessEventMoveAction(PlayerbotAI* ai) : Action(ai, "chess event move") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ChessEventAbilityAction : public Action
{
public:
    ChessEventAbilityAction(PlayerbotAI* ai) : Action(ai, "chess event ability") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Prince Malchezaar
class MalchezaarInfernalAction : public AttackAction
{
public:
    MalchezaarInfernalAction(PlayerbotAI* ai) : AttackAction(ai, "malchezaar infernal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MalchezaarEnfeebleAction : public MovementAction
{
public:
    MalchezaarEnfeebleAction(PlayerbotAI* ai) : MovementAction(ai, "malchezaar enfeeble") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Nightbane
class NightbanePositionAction : public MovementAction
{
public:
    NightbanePositionAction(PlayerbotAI* ai) : MovementAction(ai, "nightbane position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NightbaneCharredEarthAction : public MovementAction
{
public:
    NightbaneCharredEarthAction(PlayerbotAI* ai) : MovementAction(ai, "nightbane charred earth") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NightbaneAirPhaseAction : public MovementAction
{
public:
    NightbaneAirPhaseAction(PlayerbotAI* ai) : MovementAction(ai, "nightbane air phase") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NightbaneSkeletonAction : public AttackAction
{
public:
    NightbaneSkeletonAction(PlayerbotAI* ai) : AttackAction(ai, "nightbane skeleton") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Utility Actions
class KarazhanInterruptRotationAction : public Action
{
public:
    KarazhanInterruptRotationAction(PlayerbotAI* ai) : Action(ai, "karazhan interrupt rotation") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanDispelAction : public Action
{
public:
    KarazhanDispelAction(PlayerbotAI* ai) : Action(ai, "karazhan dispel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif