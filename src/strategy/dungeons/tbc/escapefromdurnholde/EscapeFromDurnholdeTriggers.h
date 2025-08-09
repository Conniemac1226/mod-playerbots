#pragma once
#include "SharedDefines.h"
#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "Unit.h"

// NPC IDs from AzerothCore source (old_hillsbrad.h)
#define NPC_THRALL              17876
#define NPC_LIEUTENANT_DRAKE    17848  
#define NPC_CAPTAIN_SKARLOC     17862
#define NPC_EPOCH_HUNTER        18096

// Spell IDs from AzerothCore boss scripts
#define SPELL_DRAKE_WHIRLWIND       31909
#define SPELL_SKARLOC_HAMMER        13005  
#define SPELL_EPOCH_SAND_BREATH     31914
#define SPELL_EPOCH_MAGIC_DISRUPTION 33834

class ThrallLowHealthTrigger : public Trigger
{
public:
    ThrallLowHealthTrigger(PlayerbotAI* ai) : Trigger(ai, "thrall low health") {}
    virtual bool IsActive();
};

class ThrallCriticalHealthTrigger : public Trigger  
{
public:
    ThrallCriticalHealthTrigger(PlayerbotAI* ai) : Trigger(ai, "thrall critical health") {}
    virtual bool IsActive();
};

class LieutenantDrakeWhirlwindTrigger : public Trigger
{
public:
    LieutenantDrakeWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "lieutenant drake whirlwind") {}
    virtual bool IsActive();
};

class CaptainSkarlocHammerOfJusticeTrigger : public Trigger
{
public:
    CaptainSkarlocHammerOfJusticeTrigger(PlayerbotAI* ai) : Trigger(ai, "captain skarloc hammer of justice") {}
    virtual bool IsActive();
};

class EpochHunterSandBreathTrigger : public Trigger
{
public:
    EpochHunterSandBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "epoch hunter sand breath") {}
    virtual bool IsActive();
};

class EpochHunterMagicDisruptionAuraTrigger : public Trigger
{
public:
    EpochHunterMagicDisruptionAuraTrigger(PlayerbotAI* ai) : Trigger(ai, "epoch hunter magic disruption aura") {}
    virtual bool IsActive();
};

class EfdReturnPositionTrigger : public Trigger
{
public:
    EfdReturnPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "efd return position") {}
    virtual bool IsActive();
};