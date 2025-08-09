#ifndef _PLAYERBOT_UNDERBOGMULTIPLIERS_H
#define _PLAYERBOT_UNDERBOGMULTIPLIERS_H

#include "Multiplier.h"

class UnderbogMultiplier : public Multiplier
{
public:
    UnderbogMultiplier(PlayerbotAI* ai) : Multiplier(ai, "underbog") {}
    float GetValue(Action* action) override;
};

#endif