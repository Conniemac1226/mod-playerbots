#ifndef _PLAYERBOT_SERPENTSHRINEMULTIPLIERS_H
#define _PLAYERBOT_SERPENTSHRINEMULTIPLIERS_H

#include "Multiplier.h"
#include "SharedDefines.h"

class HydrossResistanceMultiplier : public Multiplier
{
public:
    HydrossResistanceMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross resistance") {}
    float GetValue(Action* action) override;
};

class HydrossAddsMultiplier : public Multiplier
{
public:
    HydrossAddsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross adds") {}
    float GetValue(Action* action) override;
};

class HydrossTankMultiplier : public Multiplier
{
public:
    HydrossTankMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hydross tank") {}
    float GetValue(Action* action) override;
};

class LeotherasThreatHoldMultiplier : public Multiplier
{
public:
    LeotherasThreatHoldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "leotheras threat hold") {}
    float GetValue(Action* action) override;
};

#endif
