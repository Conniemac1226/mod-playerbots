#ifndef _PLAYERBOT_SHADOWLABYRINTHTRIGGERS_H
#define _PLAYERBOT_SHADOWLABYRINTHTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ShadowLabyrinthHelpers.h"

class HellmawCorrosiveAcidTrigger : public Trigger
{
public:
    HellmawCorrosiveAcidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hellmaw corrosive acid") {}
    bool IsActive() override;
};

class HellmawFearTrigger : public Trigger
{
public:
    HellmawFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hellmaw fear") {}
    bool IsActive() override;
};

class BlackheartInciteChaosTrigger : public Trigger
{
public:
    BlackheartInciteChaosTrigger(PlayerbotAI* botAI) : Trigger(botAI, "blackheart incite chaos") {}
    bool IsActive() override;
};

class BlackheartWarStompTrigger : public Trigger
{
public:
    BlackheartWarStompTrigger(PlayerbotAI* botAI) : Trigger(botAI, "blackheart war stomp") {}
    bool IsActive() override;
};

class BlackheartChargeTrigger : public Trigger
{
public:
    BlackheartChargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "blackheart charge") {}
    bool IsActive() override;
};

class VorpilSpreadTrigger : public Trigger
{
public:
    VorpilSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vorpil spread") {}
    bool IsActive() override;
};

class VorpilRainOfFireTrigger : public Trigger
{
public:
    VorpilRainOfFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vorpil rain of fire") {}
    bool IsActive() override;
};

class VorpilDrawShadowsTrigger : public Trigger
{
public:
    VorpilDrawShadowsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vorpil draw shadows") {}
    bool IsActive() override;
};

class MurmurSonicBoomTrigger : public Trigger
{
public:
    MurmurSonicBoomTrigger(PlayerbotAI* botAI) : Trigger(botAI, "murmur sonic boom") {}
    bool IsActive() override;
};

class MurmurResonanceTrigger : public Trigger
{
public:
    MurmurResonanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "murmur resonance") {}
    bool IsActive() override;
};

class MurmurMagneticPullTrigger : public Trigger
{
public:
    MurmurMagneticPullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "murmur magnetic pull") {}
    bool IsActive() override;
};

class MurmurThunderingStormTrigger : public Trigger
{
public:
    MurmurThunderingStormTrigger(PlayerbotAI* botAI) : Trigger(botAI, "murmur thundering storm") {}
    bool IsActive() override;
};

#endif
