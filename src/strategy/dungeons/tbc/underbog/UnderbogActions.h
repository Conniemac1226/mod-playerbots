#ifndef _PLAYERBOT_UNDERBOGACTIONS_H
#define _PLAYERBOT_UNDERBOGACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/CoilfangReservoir/underbog/the_underbog.h
enum UnderbogNpcs
{
    NPC_HUNGARFEN           = 17770,
    NPC_UNDERBOG_MUSHROOM   = 17990,
    NPC_GHAZAN              = 18105,
    NPC_SWAMPLORD_MUSELEK   = 17826,  // Standard NPC ID for Swamplord Musel'ek
    NPC_BLACK_STALKER       = 17882,  // Standard NPC ID for The Black Stalker
    NPC_WINDCALLER_CLAW     = 17827,  // Musel'ek's pet bear
    NPC_SPORE_STRIDER       = 22299   // Black Stalker adds
};

// RESEARCHED FROM: boss_hungarfen.cpp:26-37
enum HungarfenSpells
{
    SPELL_SPAWN_MUSHROOMS   = 31692,
    SPELL_DESPAWN_MUSHROOMS = 34874,
    SPELL_FOUL_SPORES       = 31673,
    SPELL_ACID_GEYSER       = 38739,
    SPELL_SHRINK            = 31691,
    SPELL_GROW              = 31698,
    SPELL_SPORE_CLOUD       = 34168
};

// RESEARCHED FROM: boss_ghazan.cpp:27-30
enum GhazanSpells
{
    SPELL_ACID_BREATH       = 34268,
    SPELL_ACID_SPIT         = 34290,
    SPELL_TAIL_SWEEP        = 34267,
    SPELL_ENRAGE            = 15716
};

// RESEARCHED FROM: boss_swamplord_muselek.cpp:23-31
enum SwamplordSpells
{
    SPELL_SHOOT               = 22907,
    SPELL_KNOCKAWAY           = 18813,
    SPELL_RAPTOR_STRIKE       = 31566,
    SPELL_MULTISHOT           = 34974,
    SPELL_THROW_FREEZING_TRAP = 31946,
    SPELL_AIMED_SHOT          = 31623,
    SPELL_HUNTERS_MARK        = 31615
};

// RESEARCHED FROM: boss_the_black_stalker.cpp:40-49
enum BlackStalkerSpells
{
    SPELL_LEVITATE                  = 31704,
    SPELL_CHAIN_LIGHTNING           = 31717,
    SPELL_STATIC_CHARGE             = 31715,
    SPELL_SUMMON_SPORE_STRIDER      = 38755,
    SPELL_LEVITATION_PULSE          = 31701,
    SPELL_SOMEONE_GRAB_ME           = 31702,
    SPELL_MAGNETIC_PULL             = 31703,
    SPELL_SUSPENSION_PRIMER         = 31720,
    SPELL_SUSPENSION                = 31719
};

// Hungarfen Actions
class HungarfenMushroomAction : public MovementAction
{
public:
    HungarfenMushroomAction(PlayerbotAI* ai) : MovementAction(ai, "avoid mushroom explosion") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class HungarfenFoulSporesAction : public MovementAction
{
public:
    HungarfenFoulSporesAction(PlayerbotAI* ai) : MovementAction(ai, "avoid foul spores") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Ghazan Actions
class GhazanAcidBreathAction : public MovementAction
{
public:
    GhazanAcidBreathAction(PlayerbotAI* ai) : MovementAction(ai, "avoid acid breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GhazanTailSweepAction : public MovementAction
{
public:
    GhazanTailSweepAction(PlayerbotAI* ai) : MovementAction(ai, "avoid tail sweep") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Swamplord Musel'ek Actions
class AttackWindcallerClawAction : public AttackAction
{
public:
    AttackWindcallerClawAction(PlayerbotAI* ai) : AttackAction(ai, "attack windcaller claw") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MuselekFreezingTrapAction : public MovementAction
{
public:
    MuselekFreezingTrapAction(PlayerbotAI* ai) : MovementAction(ai, "avoid freezing trap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MuselekHuntersMarkAction : public Action
{
public:
    MuselekHuntersMarkAction(PlayerbotAI* ai) : Action(ai, "dispel hunters mark") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// The Black Stalker Actions
class BlackStalkerLevitateAction : public MovementAction
{
public:
    BlackStalkerLevitateAction(PlayerbotAI* ai) : MovementAction(ai, "position for levitate") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackSporeStriderAction : public AttackAction
{
public:
    AttackSporeStriderAction(PlayerbotAI* ai) : AttackAction(ai, "attack spore strider") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BlackStalkerChainLightningAction : public Action
{
public:
    BlackStalkerChainLightningAction(PlayerbotAI* ai) : Action(ai, "interrupt chain lightning") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif