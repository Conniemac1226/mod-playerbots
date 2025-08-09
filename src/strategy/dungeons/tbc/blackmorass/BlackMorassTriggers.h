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