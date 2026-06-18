#ifndef _PLAYERBOT_HELLFIRERAPARTSTRIGERS_H
#define _PLAYERBOT_HELLFIRERAPARTSTRIGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "HellfireRampartsActions.h"

// Watchkeeper Gargolmar Triggers
class HellfireWatcherActiveTrigger : public Trigger
{
public:
    HellfireWatcherActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "hellfire watcher active") {}
    bool IsActive() override;
};

class GargolmarRetaliationTrigger : public Trigger
{
public:
    GargolmarRetaliationTrigger(PlayerbotAI* ai) : Trigger(ai, "gargolmar retaliation") {}
    bool IsActive() override;
};

// Omor the Unscarred Triggers
class FiendishHoundActiveTrigger : public Trigger
{
public:
    FiendishHoundActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "fiendish hound active") {}
    bool IsActive() override;
};

class OmorShadowBoltCastTrigger : public Trigger
{
public:
    OmorShadowBoltCastTrigger(PlayerbotAI* ai) : Trigger(ai, "omor shadow bolt cast") {}
    bool IsActive() override;
};

class OmorTreacherousAuraTrigger : public Trigger
{
public:
    OmorTreacherousAuraTrigger(PlayerbotAI* ai) : Trigger(ai, "omor treacherous aura") {}
    bool IsActive() override;
};

class OmorDemonicShieldTrigger : public Trigger
{
public:
    OmorDemonicShieldTrigger(PlayerbotAI* ai) : Trigger(ai, "omor demonic shield") {}
    bool IsActive() override;
};

class OmorTreacheryCastTrigger : public Trigger
{
public:
    OmorTreacheryCastTrigger(PlayerbotAI* ai) : Trigger(ai, "omor treachery cast") {}
    bool IsActive() override;
};

class OmorDebuffAvoidanceTrigger : public Trigger
{
public:
    OmorDebuffAvoidanceTrigger(PlayerbotAI* ai) : Trigger(ai, "omor debuff avoidance") {}
    bool IsActive() override;
};

class OmorClearSpreadTrigger : public Trigger
{
public:
    OmorClearSpreadTrigger(PlayerbotAI* ai) : Trigger(ai, "omor clear spread") {}
    bool IsActive() override;
};

// Vazruden & Nazan Triggers
class LiquidFireNearbyTrigger : public Trigger
{
public:
    LiquidFireNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "liquid fire nearby") {}
    bool IsActive() override;
};

class NazanConeOfFireTrigger : public Trigger
{
public:
    NazanConeOfFireTrigger(PlayerbotAI* ai) : Trigger(ai, "nazan cone of fire") {}
    bool IsActive() override;
};

class NazanLandedTrigger : public Trigger
{
public:
    NazanLandedTrigger(PlayerbotAI* ai) : Trigger(ai, "nazan landed") {}
    bool IsActive() override;
};

class VazrudenAloneTrigger : public Trigger
{
public:
    VazrudenAloneTrigger(PlayerbotAI* ai) : Trigger(ai, "vazruden alone") {}
    bool IsActive() override;
};

class NazanBellowingRoarTrigger : public Trigger
{
public:
    NazanBellowingRoarTrigger(PlayerbotAI* ai) : Trigger(ai, "nazan bellowing roar") {}
    bool IsActive() override;
};

#endif
