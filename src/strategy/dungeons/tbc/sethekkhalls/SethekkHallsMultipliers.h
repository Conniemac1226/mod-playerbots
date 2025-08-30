#ifndef _PLAYERBOT_TBCDUNGEONSHTMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONSHTMULTIPLIERS_H

#include "Multiplier.h"

class CharmingTotemMultiplier : public Multiplier
{
public:
    CharmingTotemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "charming totem") {}
    float GetValue(Action* action) override;
};

class IkissPhaseMultiplier : public Multiplier
{
public:
    IkissPhaseMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ikiss phase") {}
    float GetValue(Action* action) override;
};

#endif