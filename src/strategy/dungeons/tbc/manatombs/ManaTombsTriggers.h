#ifndef _PLAYERBOT_MANATOMBSTRIGGERS_H
#define _PLAYERBOT_MANATOMBSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

// Pandemonius Triggers
class PandemoniusDarkShellTrigger : public Trigger
{
public:
    PandemoniusDarkShellTrigger(PlayerbotAI* ai) : Trigger(ai, "dark shell active") {}
    bool IsActive() override;
};

class PandemoniusVoidBlastTrigger : public Trigger
{
public:
    PandemoniusVoidBlastTrigger(PlayerbotAI* ai) : Trigger(ai, "void blast spread") {}
    bool IsActive() override;
};

// Tavarok Triggers
class TavarokEarthquakeTrigger : public Trigger
{
public:
    TavarokEarthquakeTrigger(PlayerbotAI* ai) : Trigger(ai, "earthquake casting") {}
    bool IsActive() override;
};

class TavarokCrystalPrisonTrigger : public Trigger
{
public:
    TavarokCrystalPrisonTrigger(PlayerbotAI* ai) : Trigger(ai, "crystal prison active") {}
    bool IsActive() override;
};

class TavarokArcingSmashTrigger : public Trigger
{
public:
    TavarokArcingSmashTrigger(PlayerbotAI* ai) : Trigger(ai, "arcing smash danger") {}
    bool IsActive() override;
};

// Nexus-Prince Shaffar Triggers
class EtherealBeaconActiveTrigger : public Trigger
{
public:
    EtherealBeaconActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "ethereal beacon active") {}
    bool IsActive() override;
};

class ShaffarFrostNovaTrigger : public Trigger
{
public:
    ShaffarFrostNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "frost nova danger") {}
    bool IsActive() override;
};

class ShaffarBlinkTrigger : public Trigger
{
public:
    ShaffarBlinkTrigger(PlayerbotAI* ai) : Trigger(ai, "shaffar blinked") {}
    bool IsActive() override;
};

// Yor Triggers
class YorDoubleBreathTrigger : public Trigger
{
public:
    YorDoubleBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "double breath danger") {}
    bool IsActive() override;
};

class YorStompTrigger : public Trigger
{
public:
    YorStompTrigger(PlayerbotAI* ai) : Trigger(ai, "stomp danger") {}
    bool IsActive() override;
};

#endif