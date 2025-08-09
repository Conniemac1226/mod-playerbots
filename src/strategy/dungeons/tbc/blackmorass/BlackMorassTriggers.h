#pragma once
#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "BlackMorassActions.h"

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

class TimeLapseNearbyTrigger : public Trigger
{
public:
    TimeLapseNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "time lapse nearby") {}
    bool IsActive() override;
};

class ArcaneDischargeNearbyTrigger : public Trigger
{
public:
    ArcaneDischargeNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcane discharge nearby") {}
    bool IsActive() override;
};

class WingBuffetNearbyTrigger : public Trigger
{
public:
    WingBuffetNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wing buffet nearby") {}
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

// RESEARCHED: boss_aeonus.cpp:86 - ENRAGE increases damage
class AeonusEnragedTrigger : public Trigger
{
public:
    AeonusEnragedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aeonus enraged") {}
    bool IsActive() override;
};