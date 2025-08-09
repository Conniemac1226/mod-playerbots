#ifndef _PLAYERBOT_TBCDUNGSONSACTRIGGERS_H
#define _PLAYERBOT_TBCDUNGSONSACTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum AuchenaiCryptsIDs
{
    // NPCs - researched from boss_shirrak_the_dead_watcher.cpp
    NPC_SHIRRAK_THE_DEAD_WATCHER   = 18371,  // Boss NPC ID
    NPC_FOCUS_FIRE                 = 18374,  // ENTRY_FOCUS_FIRE from boss script line 42
    
    // Spells - researched from boss_shirrak_the_dead_watcher.cpp lines 28-36
    SPELL_INHIBIT_MAGIC            = 32264,  // Distance-based magic debuff
    SPELL_ATTRACT_MAGIC            = 32265,  // Pulls all players to boss
    SPELL_CARNIVOROUS_BITE         = 36383,  // Enhanced melee attack
    SPELL_FIERY_BLAST              = 32302,  // AoE fire damage from focus fire
    SPELL_FOCUS_FIRE_VISUAL        = 32286,  // Visual effect on focus fire creature
    SPELL_FOCUS_CAST               = 32300,  // Cast trigger for fiery blast
    SPELL_POSSESS_INSTANT          = 32830,  // Instant possession
    SPELL_POSSESS_CHANNELED        = 33401   // Channeled possession
};

class ShirrakFocusFireSpawnedTrigger : public Trigger
{
public:
    ShirrakFocusFireSpawnedTrigger(PlayerbotAI* ai) : Trigger(ai, "shirrak focus fire spawned") {}
    bool IsActive() override;
};

class ShirrakAttractMagicTrigger : public Trigger
{
public:
    ShirrakAttractMagicTrigger(PlayerbotAI* ai) : Trigger(ai, "shirrak attract magic") {}
    bool IsActive() override;
};

class ShirrakFocusFireEndedTrigger : public Trigger
{
public:
    ShirrakFocusFireEndedTrigger(PlayerbotAI* ai) : Trigger(ai, "shirrak focus fire ended") {}
    bool IsActive() override;
};

#endif