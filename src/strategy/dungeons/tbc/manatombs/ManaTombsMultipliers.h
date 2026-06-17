#ifndef _PLAYERBOT_MANATOMBSMULTIPLIERS_H
#define _PLAYERBOT_MANATOMBSMULTIPLIERS_H

#include "Multiplier.h"

class PandemoniusDarkShellMultiplier : public Multiplier
{
public:
    PandemoniusDarkShellMultiplier(PlayerbotAI* ai) : Multiplier(ai, "dark shell") {}
    float GetValue(Action* action) override;
};

class EtherealBeaconMultiplier : public Multiplier
{
public:
    EtherealBeaconMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ethereal beacon") {}
    float GetValue(Action* action) override;
};

#endif
