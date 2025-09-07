#ifndef _PLAYERBOT_BLOODFURNACEMULTIPLIERS_H
#define _PLAYERBOT_BLOODFURNACEMULTIPLIERS_H

#include "Multiplier.h"

class KelidanChannelerMultiplier : public Multiplier
{
    public:
        KelidanChannelerMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kelidan channeler") {}

    public:
        virtual float GetValue(Action* action);
};

#endif