#ifndef _PLAYERBOT_KARAZHANMULTIPLIERS_H
#define _PLAYERBOT_KARAZHANMULTIPLIERS_H

#include "Multiplier.h"

class AttumenMultiplier : public Multiplier
{
public:
    AttumenMultiplier(PlayerbotAI* ai) : Multiplier(ai, "attumen") {}
    float GetValue(Action* action) override;
};

#endif