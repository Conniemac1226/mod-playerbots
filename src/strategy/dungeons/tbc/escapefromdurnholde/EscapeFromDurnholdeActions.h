#pragma once
#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "EscapeFromDurnholdeTriggers.h"
#include "LastMovementValue.h"

class HealThrallAction : public AttackAction
{
public:
    HealThrallAction(PlayerbotAI* botAI) : AttackAction(botAI, "heal thrall") {}
    bool Execute(Event event) override;
};

class ThrallEmergencyHealAction : public AttackAction  
{
public:
    ThrallEmergencyHealAction(PlayerbotAI* botAI) : AttackAction(botAI, "thrall emergency heal") {}
    bool Execute(Event event) override;
};

class EfdAvoidWhirlwindAction : public MovementAction
{
public:
    EfdAvoidWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "efd avoid whirlwind") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class EfdReturnPositionAction : public MovementAction
{
public:
    EfdReturnPositionAction(PlayerbotAI* ai) : MovementAction(ai, "efd return position") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class DispelHammerOfJusticeAction : public CastSpellAction
{
public:
    DispelHammerOfJusticeAction(PlayerbotAI* ai) : CastSpellAction(ai, "dispel hammer of justice") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class EpochHunterPositionAction : public MovementAction
{
public:
    EpochHunterPositionAction(PlayerbotAI* ai) : MovementAction(ai, "epoch hunter position") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class CancelMagicDisruptionAction : public Action
{
public:
    CancelMagicDisruptionAction(PlayerbotAI* ai) : Action(ai, "cancel magic disruption") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};