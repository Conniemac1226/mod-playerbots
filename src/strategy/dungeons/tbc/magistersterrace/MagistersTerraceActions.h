#pragma once

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class Player;
class Unit;

namespace MagistersTerraceHelpers
{
    Unit* SelectActiveFelCrystal(Player* bot, PlayerbotAI* botAI, Unit* boss);
    bool IsKaelthasPhoenix(Unit const* unit);
    Unit* SelectKaelthasPhoenixTarget(Player* bot, PlayerbotAI* botAI, Unit* boss);
    bool IsDelrissaHelper(Unit const* unit);
    GuidVector GetDelrissaHelpersCached(PlayerbotAI* botAI, Player* bot, uint32 cacheMs = 400);
}

// Kael'thas Actions
class InterruptKaelthasPyroblastAction : public Action
{
public:
    InterruptKaelthasPyroblastAction(PlayerbotAI* ai) : Action(ai, "interrupt kaelthas pyroblast") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidGravityLapseAction : public MovementAction
{
public:
    AvoidGravityLapseAction(PlayerbotAI* ai) : MovementAction(ai, "avoid gravity lapse") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FleeArcaneSphereAction : public MovementAction
{
public:
    FleeArcaneSphereAction(PlayerbotAI* ai) : MovementAction(ai, "flee arcane sphere") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidFlamestrikeAction : public MovementAction
{
public:
    AvoidFlamestrikeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid flamestrike") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KaelthasPhoenixesAndEggsAction : public AttackAction
{
public:
    KaelthasPhoenixesAndEggsAction(PlayerbotAI* ai) : AttackAction(ai, "kaelthas phoenixes and eggs") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Vexallus Actions
class AttackPureEnergyAction : public AttackAction
{
public:
    AttackPureEnergyAction(PlayerbotAI* ai) : AttackAction(ai, "attack pure energy") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Vexallus spread
class VexallusSpreadOutAction : public MovementAction
{
public:
    VexallusSpreadOutAction(PlayerbotAI* ai) : MovementAction(ai, "spread out vexallus") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Selin Fireheart Actions
class AvoidFelExplosionAction : public MovementAction
{
public:
    AvoidFelExplosionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid fel explosion") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackFelCrystalAction : public AttackAction
{
public:
    AttackFelCrystalAction(PlayerbotAI* ai) : AttackAction(ai, "attack fel crystal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Delrissa Actions
class AttackDelrissaAddAction : public AttackAction
{
public:
    AttackDelrissaAddAction(PlayerbotAI* ai) : AttackAction(ai, "attack delrissa add") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InterruptDelrissaHelperAction : public Action
{
public:
    InterruptDelrissaHelperAction(PlayerbotAI* ai) : Action(ai, "interrupt delrissa helper") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DelrissaDispelHandlingAction : public Action
{
public:
    DelrissaDispelHandlingAction(PlayerbotAI* ai) : Action(ai, "delrissa dispel handling") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};
