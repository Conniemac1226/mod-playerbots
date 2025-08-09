#ifndef _PLAYERBOT_BLOODFURNACEACTIONS_H
#define _PLAYERBOT_BLOODFURNACEACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/HellfireCitadel/BloodFurnace/blood_furnace.h:48-55
enum BloodFurnaceNpcs
{
    NPC_THE_MAKER               = 17381,
    NPC_BROGGOK                 = 17380,
    NPC_KELIDAN                 = 17377,
    NPC_NASCENT_FEL_ORC         = 17398,
    NPC_SHADOWMOON_CHANNELER    = 17653
};

// RESEARCHED FROM: boss_the_maker.cpp:29-33
enum TheMakerSpells
{
    SPELL_EXPLODING_BEAKER      = 30925,
    SPELL_DOMINATION            = 30923
};

// RESEARCHED FROM: boss_broggok.cpp:32-38
enum BroggokSpells
{
    SPELL_BROGGOK_SLIME_SPRAY   = 30913,
    SPELL_POISON_CLOUD          = 30916,
    SPELL_POISON_BOLT           = 30917,
    SPELL_POISON                = 30914
};

// RESEARCHED FROM: boss_kelidan_the_breaker.cpp:32-40
enum KelidanSpells
{
    SPELL_CORRUPTION            = 30938,
    SPELL_EVOCATION             = 30935,
    SPELL_FIRE_NOVA             = 33132,
    SPELL_SHADOW_BOLT_VOLLEY    = 28599,
    SPELL_BURNING_NOVA          = 30940,
    SPELL_VORTEX                = 37370
};

// The Maker Actions
class MakerExplodingBeakerAction : public MovementAction
{
public:
    MakerExplodingBeakerAction(PlayerbotAI* ai) : MovementAction(ai, "avoid exploding beaker") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MakerDominationAction : public Action
{
public:
    MakerDominationAction(PlayerbotAI* ai) : Action(ai, "break domination") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Broggok Actions
class BroggokAvoidPoisonCloudAction : public MovementAction
{
public:
    BroggokAvoidPoisonCloudAction(PlayerbotAI* ai) : MovementAction(ai, "avoid poison cloud") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BroggokInterruptPoisonBoltAction : public Action
{
public:
    BroggokInterruptPoisonBoltAction(PlayerbotAI* ai) : Action(ai, "interrupt poison bolt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BroggokAvoidSlimeSprayAction : public MovementAction
{
public:
    BroggokAvoidSlimeSprayAction(PlayerbotAI* ai) : MovementAction(ai, "avoid slime spray") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Kelidan Actions
class AttackShadowmoonChannelerAction : public AttackAction
{
public:
    AttackShadowmoonChannelerAction(PlayerbotAI* ai) : AttackAction(ai, "attack shadowmoon channeler") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KelidanBurningNovaAction : public MovementAction
{
public:
    KelidanBurningNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid burning nova") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KelidanInterruptShadowBoltVolleyAction : public Action
{
public:
    KelidanInterruptShadowBoltVolleyAction(PlayerbotAI* ai) : Action(ai, "interrupt shadow bolt volley") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KelidanAvoidVortexAction : public MovementAction
{
public:
    KelidanAvoidVortexAction(PlayerbotAI* ai) : MovementAction(ai, "avoid vortex") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif