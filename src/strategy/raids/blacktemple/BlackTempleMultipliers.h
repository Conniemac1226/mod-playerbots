#ifndef _PLAYERBOT_RAIDBTMULTIPLIERS_H
#define _PLAYERBOT_RAIDBTMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"

class NajentusTargetMultiplier : public Multiplier
{
public:
    NajentusTargetMultiplier(PlayerbotAI* ai) : Multiplier(ai, "high warlord naj'entus") {}
    float GetValue(Action* action) override;
};

class NajentusImpaledMultiplier : public Multiplier
{
public:
    NajentusImpaledMultiplier(PlayerbotAI* ai) : Multiplier(ai, "najentus impaled") {}
    float GetValue(Action* action) override;
};

#endif