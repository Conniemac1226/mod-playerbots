#ifndef _PLAYERBOT_TBCDUNGEONSACACTIONS_H
#define _PLAYERBOT_TBCDUNGEONSACACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AuchenaiCryptsTriggers.h"

// Per-bot state for preventing repeated movements during Focus Fire phase - using bot GUID as key
extern std::map<ObjectGuid, bool> g_shirrak_inSafePosition;

// Spell IDs for Exarch Maladaar
const uint32 SPELL_RIBBON_OF_SOULS = 32422;
const uint32 SPELL_SOUL_SCREAM = 32421;
const uint32 SPELL_STOLEN_SOUL = 32346;
const uint32 NPC_STOLEN_SOUL = 18441;
const uint32 NPC_AVATAR = 18478;

class ShirrakFocusFireAvoidAction : public MovementAction
{
public:
    ShirrakFocusFireAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "shirrak focus fire avoid") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ShirrakReturnPositionAction : public MovementAction
{
public:
    ShirrakReturnPositionAction(PlayerbotAI* ai) : MovementAction(ai, "shirrak return position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaladaarSoulScreamAction : public MovementAction
{
public:
    MaladaarSoulScreamAction(PlayerbotAI* ai) : MovementAction(ai, "maladaar soul scream") {}
    bool Execute(Event event) override;
};

class MaladaarStolenSoulAction : public AttackAction
{
public:
    MaladaarStolenSoulAction(PlayerbotAI* ai) : AttackAction(ai, "maladaar stolen soul") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaladaarAvatarAction : public AttackAction
{
public:
    MaladaarAvatarAction(PlayerbotAI* ai) : AttackAction(ai, "maladaar avatar") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MaladaarRibbonOfSoulsAction : public Action
{
public:
    MaladaarRibbonOfSoulsAction(PlayerbotAI* ai) : Action(ai, "maladaar ribbon of souls") {}
    bool Execute(Event event) override;
};

#endif