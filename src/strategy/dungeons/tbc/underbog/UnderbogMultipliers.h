#ifndef _PLAYERBOT_UNDERBOGMULTIPLIERS_H
#define _PLAYERBOT_UNDERBOGMULTIPLIERS_H

#include "Multiplier.h"

class BlackStalkerAddMultiplier : public Multiplier
{
public:
    BlackStalkerAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "black stalker add") {}

    float GetValue(Action* action) override;
};

#endif
