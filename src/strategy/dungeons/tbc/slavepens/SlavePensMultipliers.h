#ifndef _PLAYERBOT_SLAVEPENSMULTIPLIERS_H
#define _PLAYERBOT_SLAVEPENSMULTIPLIERS_H

#include "Multiplier.h"

class SlavePensMultiplier : public Multiplier
{
public:
    SlavePensMultiplier(PlayerbotAI* ai) : Multiplier(ai, "slave pens") {}
    float GetValue(Action* action) override;
};

#endif