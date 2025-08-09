#ifndef _PLAYERBOT_TBCDUNGEONSH_MULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONSH_MULTIPLIERS_H

#include "Multiplier.h"
#include "SethekkHallsTriggers.h"

class CharmingTotemMultiplier : public Multiplier
{
public:
    CharmingTotemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "charming totem") {}

    virtual float GetValue(Action* action);
};

class IkissPhaseMultiplier : public Multiplier
{
public:
    IkissPhaseMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ikiss phase") {}

    virtual float GetValue(Action* action);
};

#endif