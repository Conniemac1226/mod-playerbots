#ifndef _PLAYERBOT_ARCATRAZTRIGGERS_H
#define _PLAYERBOT_ARCATRAZTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class ZerekethVoidZoneTrigger : public Trigger
{
public:
    ZerekethVoidZoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zereketh void zone") {}
    bool IsActive() override;
};

class ZerekethShadowNovaTrigger : public Trigger
{
public:
    ZerekethShadowNovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zereketh shadow nova") {}
    bool IsActive() override;
};

class ZerekethSeedOfCorruptionTrigger : public Trigger
{
public:
    ZerekethSeedOfCorruptionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zereketh seed of corruption") {}
    bool IsActive() override;
};

class DalliahWhirlwindTrigger : public Trigger
{
public:
    DalliahWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dalliah whirlwind") {}
    bool IsActive() override;
};

class DalliahHealTrigger : public Trigger
{
public:
    DalliahHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dalliah heal") {}
    bool IsActive() override;
};

class SoccothratesKnockAwayTrigger : public Trigger
{
public:
    SoccothratesKnockAwayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soccothrates knock away") {}
    bool IsActive() override;
};

class SoccothratesChargeTrigger : public Trigger
{
public:
    SoccothratesChargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soccothrates charge") {}
    bool IsActive() override;
};

class MellicharAddsActiveTrigger : public Trigger
{
public:
    MellicharAddsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mellichar adds active") {}
    bool IsActive() override;
};

class SkyrissIllusionTrigger : public Trigger
{
public:
    SkyrissIllusionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "skyriss illusion") {}
    bool IsActive() override;
};

class SkyrissFearTrigger : public Trigger
{
public:
    SkyrissFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "skyriss fear") {}
    bool IsActive() override;
};

class SkyrissDominationTrigger : public Trigger
{
public:
    SkyrissDominationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "skyriss domination") {}
    bool IsActive() override;
};

#endif