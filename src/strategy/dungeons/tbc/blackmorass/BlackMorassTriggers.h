#pragma once
#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "BlackMorassActions.h"

class PortalAddActiveTrigger : public Trigger
{
public:
    PortalAddActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "portal add active") {}
    bool IsActive() override;
};

class MedivhNeedsProtectionTrigger : public Trigger
{
public:
    MedivhNeedsProtectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "medivh needs protection") {}
    bool IsActive() override;
};

class AeonusCleaveNearbyTrigger : public Trigger
{
public:
    AeonusCleaveNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aeonus cleave nearby") {}
    bool IsActive() override;
};

class AeonusEngagedTrigger : public Trigger  
{
public:
    AeonusEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aeonus engaged") {}
    bool IsActive() override;
};

class ChronoLordDejaEngagedTrigger : public Trigger
{
public:
    ChronoLordDejaEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chrono lord deja engaged") {}
    bool IsActive() override;
};

class TemporusEngagedTrigger : public Trigger
{
public:
    TemporusEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "temporus engaged") {}
    bool IsActive() override;
};

class SandBreathDangerTrigger : public Trigger
{
public:
    SandBreathDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sand breath danger") {}
    bool IsActive() override;
};

class TimeLapseDangerTrigger : public Trigger
{
public:
    TimeLapseDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "time lapse danger") {}
    bool IsActive() override;
};

class ArcaneDischargeDangerTrigger : public Trigger
{
public:
    ArcaneDischargeDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcane discharge danger") {}
    bool IsActive() override;
};

class AttractionActiveTrigger : public Trigger
{
public:
    AttractionActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "attraction active") {}
    bool IsActive() override;
};

class WingBuffetDangerTrigger : public Trigger
{
public:
    WingBuffetDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wing buffet danger") {}
    bool IsActive() override;
};

class MortalWoundActiveTrigger : public Trigger
{
public:
    MortalWoundActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mortal wound active") {}
    bool IsActive() override;
};

class TemporusReflectActiveTrigger : public Trigger
{
public:
    TemporusReflectActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "temporus reflect active") {}
    bool IsActive() override;
};

// RESEARCHED: boss_temporus.cpp:54 - HASTEN buff needs dispel
class TemporusHastenActiveTrigger : public Trigger
{
public:
    TemporusHastenActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "temporus hasten active") {}
    bool IsActive() override;
};

// RESEARCHED: boss_chrono_lord_deja.cpp:58 - ARCANE_BLAST needs interrupt
class DejaArcaneBlastCastingTrigger : public Trigger
{
public:
    DejaArcaneBlastCastingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "deja arcane blast casting") {}
    bool IsActive() override;
};
