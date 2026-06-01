#ifndef _PLAYERBOT_TBCDUNGEONSHTRIGGERS_H
#define _PLAYERBOT_TBCDUNGEONSHTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum SethekkHallsIDs
{
    NPC_TIME_LOST_CONTROLLER        = 18327,  // from creature_template.sql
    NPC_CHARMING_TOTEM             = 20343,  // from creature_template.sql  
    NPC_TALON_KING_IKISS          = 18473,  // confirmed from creature_template.sql - researched from AzerothCore database
    NPC_ANZU                        = 23035,  // from boss_anzu.cpp
    NPC_BROOD_OF_ANZU              = 23132,  // from boss_anzu.cpp
    
    SPELL_SUMMON_CHARMING_TOTEM_N  = 32764,
    SPELL_CHARM_N                  = 35120,
    SPELL_BLINK_N                  = 38194,
    SPELL_ARCANE_EXPLOSION_N       = 38197,
    SPELL_ARCANE_EXPLOSION_H       = 38198,
    SPELL_ARCANE_BUBBLE            = 9438,   // from boss script line 39
    SPELL_POLYMORPH                = 38245,  // from boss script line 41
    SPELL_MANA_SHIELD              = 38151,  // from boss script line 38
    SPELL_SLOW                     = 35032,  // from boss script line 40 (heroic only)
    SPELL_SHRINK_N                 = 35013,
    SPELL_SHRINK_H                 = 38154,
};

#define SPELL_SUMMON_CHARMING_TOTEM     SPELL_SUMMON_CHARMING_TOTEM_N
#define SPELL_CHARM                     SPELL_CHARM_N
#define SPELL_BLINK                     SPELL_BLINK_N
#define SPELL_ARCANE_EXPLOSION          TBC_MODE(bot, SPELL_ARCANE_EXPLOSION_N, SPELL_ARCANE_EXPLOSION_H)
#define SPELL_SHRINK                    TBC_MODE(bot, SPELL_SHRINK_N, SPELL_SHRINK_H)

class CharmingTotemSpawnedTrigger : public Trigger
{
public:
    CharmingTotemSpawnedTrigger(PlayerbotAI* ai) : Trigger(ai, "charming totem spawned") {}
    bool IsActive() override;
};

class TimeLostControllerCastingTotemTrigger : public Trigger
{
public:
    TimeLostControllerCastingTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "time lost controller casting totem") {}
    bool IsActive() override;
};

class IkissBlinkCastTrigger : public Trigger
{
public:
    IkissBlinkCastTrigger(PlayerbotAI* ai) : Trigger(ai, "ikiss blink cast") {}
    bool IsActive() override;
};

class IkissArcaneExplosionCastTrigger : public Trigger
{
public:
    IkissArcaneExplosionCastTrigger(PlayerbotAI* ai) : Trigger(ai, "ikiss arcane explosion cast") {}
    bool IsActive() override;
};

class IkissArcaneExplosionEndedTrigger : public Trigger
{
public:
    IkissArcaneExplosionEndedTrigger(PlayerbotAI* ai) : Trigger(ai, "ikiss arcane explosion ended") {}
    bool IsActive() override;
};

class SethekkSpiritNearbyTrigger : public Trigger
{
public:
    SethekkSpiritNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "sethekk spirit nearby") {}
    bool IsActive() override;
};

class SythNoElementalsTrigger : public Trigger
{
public:
    SythNoElementalsTrigger(PlayerbotAI* ai) : Trigger(ai, "syth no elementals") {}
    bool IsActive() override;
};

class SethekkAntiFearNeededTrigger : public Trigger
{
public:
    SethekkAntiFearNeededTrigger(PlayerbotAI* ai) : Trigger(ai, "sethekk anti fear needed", 2) {}
    bool IsActive() override;
};

class IkissTankPillarPositionNeededTrigger : public Trigger
{
public:
    IkissTankPillarPositionNeededTrigger(PlayerbotAI* ai) : Trigger(ai, "ikiss tank pillar position needed", 2) {}
    bool IsActive() override;
};

class BroodOfAnzuNearbyTrigger : public Trigger
{
public:
    BroodOfAnzuNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "brood of anzu nearby") {}
    bool IsActive() override;
};

class SethekkTankAdvanceReadyTrigger : public Trigger
{
public:
    SethekkTankAdvanceReadyTrigger(PlayerbotAI* ai) : Trigger(ai, "sethekk tank advance ready", 3) {}
    bool IsActive() override;
};

#endif
