#ifndef _PLAYERBOT_MANATOMBSMULTIPLIERS_H
#define _PLAYERBOT_MANATOMBSMULTIPLIERS_H

#include "Multiplier.h"

class ManaTombsMultiplier : public Multiplier
{
public:
    ManaTombsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mana tombs") {}
    float GetValue(Action* action) override;
};

class EtherealBeaconMultiplier : public Multiplier
{
public:
    EtherealBeaconMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ethereal beacon") {}
    float GetValue(Action* action) override;
};

#endif