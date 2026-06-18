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
    NPC_SPORE_STRIDER       = 22299    // Black Stalker adds
};

enum HungarfenSpells
{
    UB_SPELL_FOUL_SPORES = 31673
};

enum GhazanSpells
{
    UB_SPELL_ACID_BREATH = 34268,
    UB_SPELL_TAIL_SWEEP = 34267
};

enum SwamplordSpells
{
    UB_SPELL_THROW_FREEZING_TRAP = 31946,
    UB_SPELL_HUNTERS_MARK = 31615
};

enum BlackStalkerSpells
{
    UB_SPELL_LEVITATE = 31704,
    UB_SPELL_CHAIN_LIGHTNING = 31717,
    UB_SPELL_STATIC_CHARGE = 31715,
    SPELL_SUSPENSION = 31719
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

class BlackStalkerStaticChargeAction : public MovementAction
{
public:
    BlackStalkerStaticChargeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid static charge") {}
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

class BlackStalkerSpreadChainLightningAction : public MovementAction
{
public:
    BlackStalkerSpreadChainLightningAction(PlayerbotAI* ai) : MovementAction(ai, "spread for chain lightning") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
