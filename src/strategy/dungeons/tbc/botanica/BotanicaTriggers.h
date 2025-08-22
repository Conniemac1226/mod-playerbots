#ifndef _PLAYERBOT_BOTANICATRIGGERS_H
#define _PLAYERBOT_BOTANICATRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class SarannisResonanceTrigger : public Trigger
{
public:
    SarannisResonanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sarannis resonance") {}
    bool IsActive() override;
};

class SarannisReinforcementsTrigger : public Trigger
{
public:
    SarannisReinforcementsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sarannis reinforcements") {}
    bool IsActive() override;
};

class FreywinnFrayersTrigger : public Trigger
{
public:
    FreywinnFrayersTrigger(PlayerbotAI* botAI) : Trigger(botAI, "freywinn frayers") {}
    bool IsActive() override;
};

class FreywinnTranquilityTrigger : public Trigger
{
public:
    FreywinnTranquilityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "freywinn tranquility") {}
    bool IsActive() override;
};

class LajAllergicReactionTrigger : public Trigger
{
public:
    LajAllergicReactionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "laj allergic reaction") {}
    bool IsActive() override;
};

class LajTeleportTrigger : public Trigger
{
public:
    LajTeleportTrigger(PlayerbotAI* botAI) : Trigger(botAI, "laj teleport") {}
    bool IsActive() override;
};

class ThorngrinSacrificeTrigger : public Trigger
{
public:
    ThorngrinSacrificeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorngrin sacrifice") {}
    bool IsActive() override;
};

class ThorngrinHellfireTrigger : public Trigger
{
public:
    ThorngrinHellfireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorngrin hellfire") {}
    bool IsActive() override;
};

class ThorngrinEnrageTrigger : public Trigger
{
public:
    ThorngrinEnrageTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorngrin enrage") {}
    bool IsActive() override;
};

class WarpSplinterWarStompTrigger : public Trigger
{
public:
    WarpSplinterWarStompTrigger(PlayerbotAI* botAI) : Trigger(botAI, "warp splinter war stomp") {}
    bool IsActive() override;
};

class WarpSplinterArcaneVolleyTrigger : public Trigger
{
public:
    WarpSplinterArcaneVolleyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "warp splinter arcane volley") {}
    bool IsActive() override;
};

#endif