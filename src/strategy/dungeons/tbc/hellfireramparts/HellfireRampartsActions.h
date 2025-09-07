#ifndef _PLAYERBOT_HELLFIRERAMPARTSACTIONS_H
#define _PLAYERBOT_HELLFIRERAMPARTSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/HellfireCitadel/HellfireRamparts/hellfire_ramparts.h
enum HellfireRampartsNpcs
{
    NPC_WATCHKEEPER_GARGOLMAR   = 17306,
    NPC_HELLFIRE_WATCHER        = 17309,
    NPC_OMOR_THE_UNSCARRED      = 17308,
    NPC_FIENDISH_HOUND          = 17540,
    NPC_VAZRUDEN_HERALD         = 17307,
    NPC_VAZRUDEN                = 17537,
    NPC_NAZAN                   = 17536,
    NPC_HELLFIRE_SENTRY         = 17517,
    NPC_LIQUID_FIRE             = 22515
};

// RESEARCHED FROM: boss_watchkeeper_gargolmar.cpp:34-39
enum GargolmarSpells
{
    SPELL_GARGOLMAR_MORTAL_WOUND = 30641,
    SPELL_SURGE                 = 34645,
    SPELL_RETALIATION           = 22857
};

// RESEARCHED FROM: boss_omor_the_unscarred.cpp:32-38
enum OmorSpells
{
    SPELL_SHADOW_BOLT           = 30686,
    SPELL_SUMMON_FIENDISH_HOUND = 30707,
    SPELL_TREACHEROUS_AURA      = 30695,
    SPELL_DEMONIC_SHIELD        = 31901
};

// RESEARCHED FROM: boss_vazruden_the_herald.cpp:35-43
enum VazrudenNazanSpells
{
    SPELL_FIREBALL              = 33793,
    SPELL_SUMMON_LIQUID_FIRE    = 31706,
    SPELL_REVENGE               = 19130,
    SPELL_BELLOWING_ROAR        = 39427,
    SPELL_CONE_OF_FIRE          = 30926
};

// Watchkeeper Gargolmar Actions
class AttackHellfireWatcherAction : public AttackAction
{
public:
    AttackHellfireWatcherAction(PlayerbotAI* ai) : AttackAction(ai, "attack hellfire watcher") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GargolmarRetaliationAction : public MovementAction
{
public:
    GargolmarRetaliationAction(PlayerbotAI* ai) : MovementAction(ai, "gargolmar retaliation") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class GargolmarSurgeAction : public MovementAction
{
public:
    GargolmarSurgeAction(PlayerbotAI* ai) : MovementAction(ai, "gargolmar surge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Omor the Unscarred Actions
class AttackFiendishHoundAction : public AttackAction
{
public:
    AttackFiendishHoundAction(PlayerbotAI* ai) : AttackAction(ai, "attack fiendish hound") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class OmorShadowBoltInterruptAction : public Action
{
public:
    OmorShadowBoltInterruptAction(PlayerbotAI* ai) : Action(ai, "interrupt omor shadow bolt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class OmorTreacherousAuraAction : public Action
{
public:
    OmorTreacherousAuraAction(PlayerbotAI* ai) : Action(ai, "omor treacherous aura") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class OmorDemonicShieldAction : public Action
{
public:
    OmorDemonicShieldAction(PlayerbotAI* ai) : Action(ai, "omor demonic shield") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};


class OmorProactiveSpreadAction : public MovementAction
{
public:
    OmorProactiveSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "omor proactive spread") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Vazruden & Nazan Actions
class NazanLiquidFireAction : public MovementAction
{
public:
    NazanLiquidFireAction(PlayerbotAI* ai) : MovementAction(ai, "avoid liquid fire") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NazanConeOfFireAction : public MovementAction
{
public:
    NazanConeOfFireAction(PlayerbotAI* ai) : MovementAction(ai, "avoid cone of fire") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackNazanFirstAction : public AttackAction
{
public:
    AttackNazanFirstAction(PlayerbotAI* ai) : AttackAction(ai, "attack nazan first") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackVazrudenAction : public AttackAction
{
public:
    AttackVazrudenAction(PlayerbotAI* ai) : AttackAction(ai, "attack vazruden") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class NazanBellowingRoarAction : public Action
{
public:
    NazanBellowingRoarAction(PlayerbotAI* ai) : Action(ai, "nazan bellowing roar") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif