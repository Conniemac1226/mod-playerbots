#ifndef _PLAYERBOT_MECHANARACTIONS_H
#define _PLAYERBOT_MECHANARACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "../actions/ChangeTargetAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/TempestKeep/Mechanar/mechanar.h
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

// RESEARCHED FROM: boss scripts
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
    SPELL_ARCANE_BLAST              = 35314,
    SPELL_DRAGONS_BREATH            = 35250,
    SPELL_RAGING_FLAMES_AREA_AURA   = 35281,
    SPELL_INFERNO                   = 35268,
    SPELL_INFERNO_DAMAGE            = 35283,
    
    // Pathaleon the Calculator
    SPELL_ARCANE_EXPLOSION          = 15453,
    SPELL_DISGRUNTLED_ANGER         = 35289,
    SPELL_ARCANE_TORRENT            = 36022,
    SPELL_MANA_TAP                  = 36021,
    SPELL_DOMINATION                = 35280,
    SPELL_FRENZY                    = 36992,
    SPELL_SUICIDE                   = 35301
};

// Per-bot state management for Polarity Shift
extern std::map<ObjectGuid, uint32> g_capacitus_lastPolarityTime;
extern std::map<ObjectGuid, bool> g_capacitus_hasPositive;
extern std::map<ObjectGuid, bool> g_capacitus_hasNegative;

// Per-bot state for Raging Flames
extern std::map<ObjectGuid, ObjectGuid> g_sepethrea_targetedByFlames;
extern std::map<ObjectGuid, uint32> g_sepethrea_lastFlamesTime;

// ========== MECHANO LORD CAPACITUS ACTIONS ==========

// RESEARCHED: boss_mechano_lord_capacitus.cpp:89-96 - Reflective shields on Normal
class CapacitusReflectiveShieldAction : public Action
{
public:
    CapacitusReflectiveShieldAction(PlayerbotAI* ai) : Action(ai, "handle reflective shield") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_mechano_lord_capacitus.cpp:78-82 - Polarity Shift on Heroic
class CapacitusPolarityShiftAction : public MovementAction
{
public:
    CapacitusPolarityShiftAction(PlayerbotAI* ai) : MovementAction(ai, "handle polarity shift") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_mechano_lord_capacitus.cpp:64-70 - Nether Charges spawn randomly
class CapacitusNetherChargeAction : public AttackAction
{
public:
    CapacitusNetherChargeAction(PlayerbotAI* ai) : AttackAction(ai, "attack nether charge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_mechano_lord_capacitus.cpp:61 - Headcrack on tank
class CapacitusPositionAction : public MovementAction
{
public:
    CapacitusPositionAction(PlayerbotAI* ai) : MovementAction(ai, "capacitus position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// ========== NETHERMANCER SEPETHREA ACTIONS ==========

// FIXED: Raging Flames are IMMUNE - must be KITED continuously, not fought
class SepethreaRagingFlamesAction : public MovementAction
{
public:
    SepethreaRagingFlamesAction(PlayerbotAI* ai) : MovementAction(ai, "flee raging flames") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_nethermancer_sepethrea.cpp:72 - Dragon's Breath frontal cone
class SepethreaDragonsBreathAction : public MovementAction
{
public:
    SepethreaDragonsBreathAction(PlayerbotAI* ai) : MovementAction(ai, "avoid dragons breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_nethermancer_sepethrea.cpp:153 - Inferno AoE from Raging Flames
class SepethreaInfernoAction : public MovementAction
{
public:
    SepethreaInfernoAction(PlayerbotAI* ai) : MovementAction(ai, "avoid inferno") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_nethermancer_sepethrea.cpp:63-68 - Arcane Blast with threat reduction
class SepethreaArcaneBlastAction : public Action
{
public:
    SepethreaArcaneBlastAction(PlayerbotAI* ai) : Action(ai, "handle arcane blast") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// FIXED: Raging Flames are IMMUNE to damage - DPS should target boss while kiting
class SepethreaTargetElementalAction : public ai::ChangeTargetAction
{
public:
    SepethreaTargetElementalAction(PlayerbotAI* ai) : ChangeTargetAction(ai, "target raging flames") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// ========== PATHALEON THE CALCULATOR ACTIONS ==========

// RESEARCHED: boss_pathaleon_the_calculator.cpp:117-122 - Mind Control (Domination)
class PathaleonDominationAction : public Action
{
public:
    PathaleonDominationAction(PlayerbotAI* ai) : Action(ai, "handle domination") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_pathaleon_the_calculator.cpp:96-101 - Summons Nether Wraiths
class PathaleonNetherWraithAction : public AttackAction
{
public:
    PathaleonNetherWraithAction(PlayerbotAI* ai) : AttackAction(ai, "attack nether wraith") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_pathaleon_the_calculator.cpp:113-115 - Arcane Torrent AoE
class PathaleonArcaneTorrentAction : public MovementAction
{
public:
    PathaleonArcaneTorrentAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane torrent") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_pathaleon_the_calculator.cpp:84-90 - Enrage at 20% with Suicide
class PathaleonEnrageAction : public MovementAction
{
public:
    PathaleonEnrageAction(PlayerbotAI* ai) : MovementAction(ai, "handle pathaleon enrage") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_pathaleon_the_calculator.cpp:131-135 - Arcane Explosion on Heroic
class PathaleonArcaneExplosionAction : public MovementAction
{
public:
    PathaleonArcaneExplosionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane explosion") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_pathaleon_the_calculator.cpp:104-109 - Mana Tap on mana users
class PathaleonManaTapAction : public Action
{
public:
    PathaleonManaTapAction(PlayerbotAI* ai) : Action(ai, "handle mana tap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif