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

#endif