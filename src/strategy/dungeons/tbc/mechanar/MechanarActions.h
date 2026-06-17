#ifndef _PLAYERBOT_MECHANARACTIONS_H
#define _PLAYERBOT_MECHANARACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

enum MechanarNpcs
{
    // Bosses
    NPC_MECHANO_LORD_CAPACITUS     = 19219,
    NPC_NETHERMANCER_SEPETHREA     = 19221,
    NPC_PATHALEON_THE_CALCULATOR   = 19220,
    NPC_GATEWATCHER_GYROKILL       = 19218,
    NPC_GATEWATCHER_IRONHAND       = 19710,
    
    // Adds
    NPC_NETHER_CHARGE              = 20405,
    NPC_RAGING_FLAMES              = 20481,
    NPC_NETHER_WRAITH              = 21062,
    
    // Trash
    NPC_SUNSEEKER_ASTROMAGE        = 19168,
    NPC_SUNSEEKER_ENGINEER         = 20988,
    NPC_BLOODWARDER_CENTURION      = 19510,
    NPC_BLOODWARDER_PHYSICIAN      = 20990,
    NPC_TEMPEST_KEEPER_DESTROYER   = 19735
};

enum MechanarSpells
{
    // Mechano Lord Capacitus
    SPELL_HEADCRACK                 = 35161,
    SPELL_REFLECTIVE_MAGIC_SHIELD   = 35158,
    SPELL_REFLECTIVE_DAMAGE_SHIELD  = 35159,
    SPELL_POLARITY_SHIFT            = 39096,
    SPELL_POSITIVE_POLARITY         = 39088,
    SPELL_NEGATIVE_POLARITY         = 39091,
    SPELL_POSITIVE_CHARGE           = 39090,
    SPELL_NEGATIVE_CHARGE           = 39093,
    SPELL_NETHER_CHARGE_PASSIVE     = 35150,
    
    // Nethermancer Sepethrea
    SPELL_FROST_ATTACK              = 45196,
    MECH_SPELL_ARCANE_BLAST         = 35314,
    MECH_SPELL_DRAGONS_BREATH       = 35250,
    SPELL_INFERNO                   = 35268,

    // Pathaleon the Calculator
    MECH_SPELL_ARCANE_EXPLOSION     = 15453,
    MECH_SPELL_DISGRUNTLED_ANGER    = 35289,
    MECH_SPELL_ARCANE_TORRENT       = 36022,
    MECH_SPELL_MANA_TAP             = 36021,
    MECH_SPELL_DOMINATION           = 35280,
    MECH_SPELL_FRENZY               = 36992,
    MECH_SPELL_SUICIDE              = 35301
};

// ========== MECHANO LORD CAPACITUS ACTIONS ==========

class CapacitusReflectiveShieldAction : public Action
{
public:
    CapacitusReflectiveShieldAction(PlayerbotAI* ai) : Action(ai, "handle reflective shield") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class CapacitusPolarityShiftAction : public MovementAction
{
public:
    CapacitusPolarityShiftAction(PlayerbotAI* ai) : MovementAction(ai, "handle polarity shift") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class CapacitusNetherChargeAction : public AttackAction
{
public:
    CapacitusNetherChargeAction(PlayerbotAI* ai) : AttackAction(ai, "attack nether charge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class CapacitusPositionAction : public MovementAction
{
public:
    CapacitusPositionAction(PlayerbotAI* ai) : MovementAction(ai, "capacitus position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// ========== NETHERMANCER SEPETHREA ACTIONS ==========

class SepethreaRagingFlamesAction : public MovementAction
{
public:
    SepethreaRagingFlamesAction(PlayerbotAI* ai) : MovementAction(ai, "flee raging flames") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SepethreaInfernoAvoidanceAction : public MovementAction
{
public:
    SepethreaInfernoAvoidanceAction(PlayerbotAI* ai) : MovementAction(ai, "avoid raging flames inferno") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SepethreaFireTrailAvoidanceAction : public MovementAction
{
public:
    SepethreaFireTrailAvoidanceAction(PlayerbotAI* ai) : MovementAction(ai, "avoid raging flames fire trail") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SepethreaDragonsBreathAction : public MovementAction
{
public:
    SepethreaDragonsBreathAction(PlayerbotAI* ai) : MovementAction(ai, "avoid dragons breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SepethreaArcaneBlastAction : public Action
{
public:
    SepethreaArcaneBlastAction(PlayerbotAI* ai) : Action(ai, "handle arcane blast") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SepethreaAvoidRagingFlamesAction : public MovementAction
{
public:
    SepethreaAvoidRagingFlamesAction(PlayerbotAI* ai) : MovementAction(ai, "avoid raging flames creature") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// ========== PATHALEON THE CALCULATOR ACTIONS ==========

class PathaleonDominationAction : public Action
{
public:
    PathaleonDominationAction(PlayerbotAI* ai) : Action(ai, "handle domination") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PathaleonNetherWraithAction : public AttackAction
{
public:
    PathaleonNetherWraithAction(PlayerbotAI* ai) : AttackAction(ai, "attack nether wraith") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PathaleonArcaneTorrentAction : public MovementAction
{
public:
    PathaleonArcaneTorrentAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane torrent") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PathaleonEnrageAction : public MovementAction
{
public:
    PathaleonEnrageAction(PlayerbotAI* ai) : MovementAction(ai, "handle pathaleon enrage") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PathaleonArcaneExplosionAction : public MovementAction
{
public:
    PathaleonArcaneExplosionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane explosion") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PathaleonManaTapAction : public Action
{
public:
    PathaleonManaTapAction(PlayerbotAI* ai) : Action(ai, "handle mana tap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
