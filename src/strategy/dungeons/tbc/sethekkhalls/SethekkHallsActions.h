#ifndef _PLAYERBOT_TBCDUNGEONSHACTIONS_H
#define _PLAYERBOT_TBCDUNGEONSHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SethekkHallsTriggers.h"

// NPC IDs
#define NPC_CHARMING_TOTEM       20343
#define NPC_TIME_LOST_CONTROLLER 20691
#define NPC_DARKWEAVER_SYTH      18472
#define NPC_TALON_KING_IKISS     18473
#define NPC_SETHEKK_SPIRIT       18703
#define NPC_ANZU                 23035
#define NPC_BROOD_OF_ANZU        23132

// Darkweaver Syth elemental adds
#define NPC_SYTH_FIRE_ELEMENTAL    19203
#define NPC_SYTH_FROST_ELEMENTAL   19204  
#define NPC_SYTH_ARCANE_ELEMENTAL  19205
#define NPC_SYTH_SHADOW_ELEMENTAL  19206

// Anzu spell IDs
#define SPELL_PARALYZING_SCREECH   40184
#define SPELL_SPELL_BOMB            40303
#define SPELL_ANZU_CYCLONE          40321  // Renamed to avoid conflict with ICC
#define SPELL_BANISH_SELF           42354

// Spell IDs
#define SPELL_SUMMON_TOTEM          32764  // Time-Lost Controller totem summon
#define SPELL_ARCANE_BUBBLE         9438   // Ikiss arcane explosion prep
#define SPELL_BLINK_N               38194  // Ikiss blink

// Search ranges
#define SEARCH_RANGE_LARGE          50.0f  // General enemy search
#define SEARCH_RANGE_MEDIUM         30.0f  // Controller interrupt range
#define SEARCH_RANGE_SMALL          20.0f  // Spirit danger range
#define SETHEKK_SPIRIT_SAFE_RANGE    8.5f
#define SETHEKK_SPIRIT_FLEE_STEP     5.0f
#define SYTH_STACK_RANGE             6.0f
#define AUTO_PULL_SEARCH_RANGE      45.0f
#define AUTO_PULL_GROUP_RANGE       35.0f
#define AUTO_PULL_HEALER_MANA_PCT   45.0f
#define AUTO_PULL_MEMBER_HP_PCT     60.0f
#define AUTO_PULL_TANK_HP_PCT       75.0f

const Position IKISS_LOS_SAFE_POSITIONS[4] =
{
    Position(18.69f, 316.45f, 26.34f),  // Southwest pillar safe spot
    Position(75.19f, 314.44f, 28.5f),   // Northwest pillar safe spot  
    Position(14.98f, 257.81f, 26.50f),  // Southeast pillar safe spot
    Position(73.11f, 259.09f, 27.74f)   // Northeast pillar safe spot
};

extern std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
extern std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

class MarkCharmingTotemAction : public Action
{
public:
    MarkCharmingTotemAction(PlayerbotAI* ai) : Action(ai, "mark charming totem") {}
    bool Execute(Event event) override;

private:
    void UpdateSkullMarker(Unit* totem);
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

class FleeSpiritAction : public MovementAction
{
public:
    FleeSpiritAction(PlayerbotAI* ai) : MovementAction(ai, "flee sethekk spirit") {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Unit* FindThreateningSpirit(Player* bot) const;
};

class AttackBroodOfAnzuAction : public AttackAction
{
public:
    AttackBroodOfAnzuAction(PlayerbotAI* ai) : AttackAction(ai, "attack brood of anzu") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ContinueFightWithCharmedAllyAction : public AttackAction
{
public:
    ContinueFightWithCharmedAllyAction(PlayerbotAI* ai) : AttackAction(ai, "continue fight with charmed ally") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackSythElementalsAction : public AttackAction
{
public:
    AttackSythElementalsAction(PlayerbotAI* ai) : AttackAction(ai, "attack syth elementals") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class StackForSythAction : public MovementAction
{
public:
    StackForSythAction(PlayerbotAI* ai) : MovementAction(ai, "stack for syth") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class SethekkTankAdvancePullAction : public AttackAction
{
public:
    SethekkTankAdvancePullAction(PlayerbotAI* ai) : AttackAction(ai, "sethekk tank advance pull") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};


#endif
