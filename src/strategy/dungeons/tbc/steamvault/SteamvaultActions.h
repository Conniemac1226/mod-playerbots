#pragma once

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Hydromancer Thespia Actions
class AvoidLightningCloudAction : public MovementAction
{
public:
    AvoidLightningCloudAction(PlayerbotAI* ai) : MovementAction(ai, "avoid lightning cloud") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DispelLungBurstAction : public Action
{
public:
    DispelLungBurstAction(PlayerbotAI* ai) : Action(ai, "dispel lung burst") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackWaterElementalAction : public AttackAction
{
public:
    AttackWaterElementalAction(PlayerbotAI* ai) : AttackAction(ai, "attack water elemental") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Mekgineer Steamrigger Actions
class DispelShrinkRayAction : public Action
{
public:
    DispelShrinkRayAction(PlayerbotAI* ai) : Action(ai, "dispel shrink ray") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidSawBladeAction : public MovementAction
{
public:
    AvoidSawBladeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid saw blade") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RemoveElectrifiedNetAction : public Action
{
public:
    RemoveElectrifiedNetAction(PlayerbotAI* ai) : Action(ai, "remove electrified net") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackSteamriggerMechanicAction : public AttackAction
{
public:
    AttackSteamriggerMechanicAction(PlayerbotAI* ai) : AttackAction(ai, "attack steamrigger mechanic") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Warlord Kalithresh Actions
class StopCastingSpellReflectionAction : public Action
{
public:
    StopCastingSpellReflectionAction(PlayerbotAI* ai) : Action(ai, "stop casting spell reflection") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class HealImpaleTargetAction : public Action
{
public:
    HealImpaleTargetAction(PlayerbotAI* ai) : Action(ai, "heal impale target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackNagaDistillerAction : public AttackAction
{
public:
    AttackNagaDistillerAction(PlayerbotAI* ai) : AttackAction(ai, "attack naga distiller") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InterruptDistillerChannelAction : public Action
{
public:
    InterruptDistillerChannelAction(PlayerbotAI* ai) : Action(ai, "interrupt distiller channel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};