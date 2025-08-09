#ifndef _PLAYERBOT_KARAZHANTRIGGERS_H
#define _PLAYERBOT_KARAZHANTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum KarazhanIDs
{
    // Attumen the Huntsman
    NPC_MIDNIGHT                = 16151,
    NPC_ATTUMEN_UNMOUNTED      = 15550,
    NPC_ATTUMEN_MOUNTED        = 16152,
    
    SPELL_SHADOWCLEAVE         = 29832,
    SPELL_INTANGIBLE_PRESENCE  = 29833,
    SPELL_KNOCKDOWN            = 29711,
    SPELL_CHARGE               = 29847,
    SPELL_MOUNT                = 29770,
    SPELL_SUMMON_ATTUMEN       = 29714
};

// Attumen the Huntsman triggers
class AttumenEngagedTrigger : public Trigger
{
public:
    AttumenEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen engaged") {}
    bool IsActive() override;
};

class AttumenMountedTrigger : public Trigger
{
public:
    AttumenMountedTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen mounted") {}
    bool IsActive() override;
};

class AttumenChargeDangerTrigger : public Trigger
{
public:
    AttumenChargeDangerTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen charge danger") {}
    bool IsActive() override;
};

class AttumenShadowcleaveTrigger : public Trigger
{
public:
    AttumenShadowcleaveTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen shadowcleave") {}
    bool IsActive() override;
};

#endif