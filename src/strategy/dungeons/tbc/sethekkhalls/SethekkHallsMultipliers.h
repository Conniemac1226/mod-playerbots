#ifndef _PLAYERBOT_TBCDUNGEONSHTMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONSHTMULTIPLIERS_H

#include "Multiplier.h"

// REMOVED: CharmingTotemMultiplier (ICC Pattern: skull marking replaces multiplier)

class IkissPhaseMultiplier : public Multiplier
{
public:
    IkissPhaseMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ikiss phase") {}
    float GetValue(Action* action) override;
};

class BroodOfAnzuMultiplier : public Multiplier
{
public:
    BroodOfAnzuMultiplier(PlayerbotAI* ai) : Multiplier(ai, "brood of anzu") {}
    float GetValue(Action* action) override;
};

#endif