#ifndef _PLAYERBOT_MAGTHERIDONMULTIPLIERS_H
#define _PLAYERBOT_MAGTHERIDONMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "MagtheridonActions.h"

// Increase priority of cube clicking during Blast Nova
class BlastNovaMultiplier : public Multiplier
{
public:
    BlastNovaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "blast nova") {}
    float GetValue(Action* action) override;
};

// Increase priority of interrupts on channelers
class ChannelerInterruptMultiplier : public Multiplier
{
public:
    ChannelerInterruptMultiplier(PlayerbotAI* ai) : Multiplier(ai, "channeler interrupt") {}
    float GetValue(Action* action) override;
};

// Increase priority of movement during dangerous phases
class MagtheridonMovementMultiplier : public Multiplier
{
public:
    MagtheridonMovementMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magtheridon movement") {}
    float GetValue(Action* action) override;
};

// Prioritize add targeting
class MagtheridonAddsMultiplier : public Multiplier
{
public:
    MagtheridonAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magtheridon adds") {}
    float GetValue(Action* action) override;
};

// Tank priority adjustments
class MagtheridonTankMultiplier : public Multiplier
{
public:
    MagtheridonTankMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magtheridon tank") {}
    float GetValue(Action* action) override;
};

// Healer priority adjustments
class MagtheridonHealerMultiplier : public Multiplier
{
public:
    MagtheridonHealerMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magtheridon healer") {}
    float GetValue(Action* action) override;
};

#endif