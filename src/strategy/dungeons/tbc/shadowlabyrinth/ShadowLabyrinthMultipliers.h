#ifndef _PLAYERBOT_SHADOWLABYRINTHMULTIPLIERS_H
#define _PLAYERBOT_SHADOWLABYRINTHMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class HellmawMultiplier : public Multiplier
{
public:
    HellmawMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ambassador hellmaw") {}
    float GetValue(Action* action) override;
};

class BlackheartMultiplier : public Multiplier
{
public:
    BlackheartMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "blackheart the inciter") {}
    float GetValue(Action* action) override;
};

class VorpilMultiplier : public Multiplier
{
public:
    VorpilMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "grandmaster vorpil") {}
    float GetValue(Action* action) override;
};

class MurmurMultiplier : public Multiplier
{
public:
    MurmurMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "murmur") {}
    float GetValue(Action* action) override;
};

#endif
