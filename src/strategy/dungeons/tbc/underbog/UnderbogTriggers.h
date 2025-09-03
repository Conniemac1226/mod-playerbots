#ifndef _PLAYERBOT_UNDERBOGTRIGGERS_H
#define _PLAYERBOT_UNDERBOGTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

// Hungarfen Triggers
class HungarfenMushroomNearbyTrigger : public Trigger
{
public:
    HungarfenMushroomNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "mushroom nearby") {}
    bool IsActive() override;
};

class HungarfenFoulSporesTrigger : public Trigger
{
public:
    HungarfenFoulSporesTrigger(PlayerbotAI* ai) : Trigger(ai, "foul spores active") {}
    bool IsActive() override;
};

// Ghazan Triggers
class GhazanAcidBreathTrigger : public Trigger
{
public:
    GhazanAcidBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "acid breath casting") {}
    bool IsActive() override;
};

class GhazanTailSweepTrigger : public Trigger
{
public:
    GhazanTailSweepTrigger(PlayerbotAI* ai) : Trigger(ai, "tail sweep danger") {}
    bool IsActive() override;
};

// Swamplord Musel'ek Triggers
class WindcallerClawActiveTrigger : public Trigger
{
public:
    WindcallerClawActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "windcaller claw active") {}
    bool IsActive() override;
};

class MuselekFreezingTrapTrigger : public Trigger
{
public:
    MuselekFreezingTrapTrigger(PlayerbotAI* ai) : Trigger(ai, "freezing trap incoming") {}
    bool IsActive() override;
};

class MuselekHuntersMarkTrigger : public Trigger
{
public:
    MuselekHuntersMarkTrigger(PlayerbotAI* ai) : Trigger(ai, "hunters mark active") {}
    bool IsActive() override;
};

// The Black Stalker Triggers
class BlackStalkerLevitateTrigger : public Trigger
{
public:
    BlackStalkerLevitateTrigger(PlayerbotAI* ai) : Trigger(ai, "levitate active") {}
    bool IsActive() override;
};

class SporeStriderActiveTrigger : public Trigger
{
public:
    SporeStriderActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "spore strider active") {}
    bool IsActive() override;
};

class BlackStalkerChainLightningTrigger : public Trigger
{
public:
    BlackStalkerChainLightningTrigger(PlayerbotAI* ai) : Trigger(ai, "chain lightning casting") {}
    bool IsActive() override;
};

class BlackStalkerEncounterActiveTrigger : public Trigger
{
public:
    BlackStalkerEncounterActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "black stalker encounter active") {}
    bool IsActive() override;
};

#endif