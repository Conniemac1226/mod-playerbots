#ifndef _PLAYERBOT_TBCDUNGEONSACMULTIPLIERS_H
#define _PLAYERBOT_TBCDUNGEONSACMULTIPLIERS_H

#include "Multiplier.h"

class ShirrakFocusFireMultiplier : public Multiplier
{
public:
    ShirrakFocusFireMultiplier(PlayerbotAI* ai) : Multiplier(ai, "shirrak focus fire avoid") {}

public:
    float GetValue(Action* action) override;
};

#endif