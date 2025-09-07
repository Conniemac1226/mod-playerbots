#ifndef _PLAYERBOT_HELLFIRERAMPARTSMULTIPLIERS_H
#define _PLAYERBOT_HELLFIRERAMPARTSMULTIPLIERS_H

#include "Multiplier.h"

class OmorAddMultiplier : public Multiplier
{
    public:
        OmorAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "omor add") {}

    public:
        virtual float GetValue(Action* action);
};

#endif