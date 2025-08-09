#ifndef _PLAYERBOT_TBCDUNGEONSHACTIONS_H
#define _PLAYERBOT_TBCDUNGEONSHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SethekkHallsTriggers.h"

// TESTED: LoS safe positions behind pillars - coordinates confirmed to break LoS from boss
const Position IKISS_LOS_SAFE_POSITIONS[4] =
{
    Position(18.69f, 316.45f, 26.34f),  // Southwest pillar safe spot
    Position(75.19f, 314.44f, 28.5f),   // Northwest pillar safe spot  
    Position(14.98f, 257.81f, 26.50f),  // Southeast pillar safe spot
    Position(73.11f, 259.09f, 27.74f)   // Northeast pillar safe spot
};

// Per-bot state for preventing repeated movements - using bot GUID as key
extern std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
extern std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

class AttackCharmingTotemAction : public AttackAction
{
public:
    AttackCharmingTotemAction(PlayerbotAI* ai) : AttackAction(ai, "attack charming totem") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InterruptControllerAction : public Action
{
public:
    InterruptControllerAction(PlayerbotAI* ai) : Action(ai, "interrupt controller") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IkissMoveAwayAction : public MovementAction
{
public:
    IkissMoveAwayAction(PlayerbotAI* ai) : MovementAction(ai, "ikiss move away") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IkissReturnPositionAction : public MovementAction
{
public:
    IkissReturnPositionAction(PlayerbotAI* ai) : MovementAction(ai, "ikiss return position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif