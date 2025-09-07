#ifndef _PLAYERBOT_SLAVEPENSMULTIPLIERS_H
#define _PLAYERBOT_SLAVEPENSMULTIPLIERS_H

#include "Multiplier.h"

class SlavePensMultiplier : public Multiplier
{
public:
    SlavePensMultiplier(PlayerbotAI* ai) : Multiplier(ai, "slave pens") {}
    float GetValue(Action* action) override;
};

class MennuTotemMultiplier : public Multiplier
{
public:
    MennuTotemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mennu totem") {}
    float GetValue(Action* action) override;
};

#endif