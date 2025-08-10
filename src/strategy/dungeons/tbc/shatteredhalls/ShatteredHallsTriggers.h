#ifndef _PLAYERBOT_SHATTEREDHALLSTRIGGERS_H
#define _PLAYERBOT_SHATTEREDHALLSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class NethekurseShadowFissureTrigger : public Trigger
{
public:
    NethekurseShadowFissureTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nethekurse shadow fissure") {}
    bool IsActive() override;
};

class NethekurseDarkSpinTrigger : public Trigger
{
public:
    NethekurseDarkSpinTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nethekurse dark spin") {}
    bool IsActive() override;
};

class NethekursePeonsTrigger : public Trigger
{
public:
    NethekursePeonsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nethekurse peons") {}
    bool IsActive() override;
};

class OmroggBlastWaveTrigger : public Trigger
{
public:
    OmroggBlastWaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omrogg blast wave") {}
    bool IsActive() override;
};

class OmroggBurningMaulTrigger : public Trigger
{
public:
    OmroggBurningMaulTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omrogg burning maul") {}
    bool IsActive() override;
};

class KargathBladeDanceTrigger : public Trigger
{
public:
    KargathBladeDanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kargath blade dance") {}
    bool IsActive() override;
};

class KargathAssassinsTrigger : public Trigger
{
public:
    KargathAssassinsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kargath assassins") {}
    bool IsActive() override;
};

class FlameArrowGauntletTrigger : public Trigger
{
public:
    FlameArrowGauntletTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flame arrow gauntlet") {}
    bool IsActive() override;
};

#endif