#ifndef _PLAYERBOT_GRUULACTIONS_H
#define _PLAYERBOT_GRUULACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

enum GruulNPCs
{
    // High King Maulgar and Council
    NPC_HIGH_KING_MAULGAR      = 18831,
    NPC_KROSH_FIREHAND          = 18832,
    NPC_OLM_THE_SUMMONER        = 18834,
    NPC_KIGGLER_THE_CRAZED      = 18835,
    NPC_BLINDEYE_THE_SEER       = 18836,
    NPC_WILD_FEL_STALKER        = 18847,
    
    // Gruul
    NPC_GRUUL_THE_DRAGONKILLER  = 19044,
    NPC_INVISIBLE_TRACTOR_BEAM  = 19224
};

enum GruulSpells
{
    // Gruul the Dragonkiller
    GRUUL_SPELL_GROWTH                = 36300,
    GRUUL_SPELL_CAVE_IN               = 36240,
    GRUUL_SPELL_GROUND_SLAM           = 33525,
    GRUUL_SPELL_REVERBERATION         = 36297,
    GRUUL_SPELL_HURTFUL_STRIKE        = 33813,
    GRUUL_SPELL_SHATTER               = 33654,
    GRUUL_SPELL_SHATTER_EFFECT        = 33671,
    GRUUL_SPELL_STONED                = 33652,
    GRUUL_SPELL_LOOK_AROUND           = 33965,
    GRUUL_SPELL_TRACTOR_BEAM_PULL     = 33497,
    
    // High King Maulgar
    MAULGAR_SPELL_ARCING_SMASH        = 39144,
    MAULGAR_SPELL_MIGHTY_BLOW         = 33230,
    MAULGAR_SPELL_WHIRLWIND           = 33238,
    MAULGAR_SPELL_BERSERKER_C         = 26561,
    MAULGAR_SPELL_ROAR                = 16508,
    MAULGAR_SPELL_FLURRY              = 33232,
    
    // Olm the Summoner
    OLM_SPELL_DARK_DECAY              = 33129,
    OLM_SPELL_DEATH_COIL              = 33130,
    OLM_SPELL_SUMMON_WFH              = 33131,
    
    // Kiggler the Crazed
    KIGGLER_SPELL_GREATER_POLYMORPH   = 33173,
    KIGGLER_SPELL_LIGHTNING_BOLT      = 36152,
    KIGGLER_SPELL_ARCANE_SHOCK        = 33175,
    KIGGLER_SPELL_ARCANE_EXPLOSION    = 33237,
    
    // Blindeye the Seer
    BLINDEYE_SPELL_GREATER_PW_SHIELD  = 33147,
    BLINDEYE_SPELL_HEAL               = 33144,
    BLINDEYE_SPELL_PRAYER_OH          = 33152,
    
    // Krosh Firehand
    KROSH_SPELL_GREATER_FIREBALL      = 33051,
    KROSH_SPELL_SPELLSHIELD           = 33054,
    KROSH_SPELL_BLAST_WAVE            = 33061
};

// Gruul the Dragonkiller Actions
class GruulGroundSlamAction : public MovementAction
{
public:
    GruulGroundSlamAction(PlayerbotAI* ai) : MovementAction(ai, "gruul ground slam") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulShatterPositionAction : public MovementAction
{
public:
    GruulShatterPositionAction(PlayerbotAI* ai) : MovementAction(ai, "gruul shatter position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulCaveInAction : public MovementAction
{
public:
    GruulCaveInAction(PlayerbotAI* ai) : MovementAction(ai, "gruul cave in") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulHurtfulStrikeAction : public MovementAction
{
public:
    GruulHurtfulStrikeAction(PlayerbotAI* ai) : MovementAction(ai, "gruul hurtful strike") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// High King Maulgar Actions
class MaulgarFocusTargetAction : public AttackAction
{
public:
    MaulgarFocusTargetAction(PlayerbotAI* ai) : AttackAction(ai, "maulgar focus target") {}
    bool Execute(Event event) override;
};

class MaulgarPositionAction : public MovementAction
{
public:
    MaulgarPositionAction(PlayerbotAI* ai) : MovementAction(ai, "maulgar position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaulgarWhirlwindAction : public MovementAction
{
public:
    MaulgarWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "maulgar whirlwind") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaulgarArcingSmashAction : public MovementAction
{
public:
    MaulgarArcingSmashAction(PlayerbotAI* ai) : MovementAction(ai, "maulgar arcing smash") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Council Member Actions
class KroshSpellstealAction : public Action
{
public:
    KroshSpellstealAction(PlayerbotAI* ai) : Action(ai, "krosh spellsteal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KigglerPolymorphAction : public Action
{
public:
    KigglerPolymorphAction(PlayerbotAI* ai) : Action(ai, "kiggler polymorph") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class OlmWildFelStalkerAction : public AttackAction
{
public:
    OlmWildFelStalkerAction(PlayerbotAI* ai) : AttackAction(ai, "olm wild fel stalker") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BlindeyeInterruptAction : public Action
{
public:
    BlindeyeInterruptAction(PlayerbotAI* ai) : Action(ai, "blindeye interrupt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AoE Avoidance Actions
class KroshBlastWaveAvoidAction : public MovementAction
{
public:
    KroshBlastWaveAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "krosh blast wave avoid") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KigglerArcaneExplosionAvoidAction : public MovementAction
{
public:
    KigglerArcaneExplosionAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "kiggler arcane explosion avoid") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Utility Actions
class GruulTankSwapAction : public Action
{
public:
    GruulTankSwapAction(PlayerbotAI* ai) : Action(ai, "gruul tank swap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulDispelAction : public Action
{
public:
    GruulDispelAction(PlayerbotAI* ai) : Action(ai, "gruul dispel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
