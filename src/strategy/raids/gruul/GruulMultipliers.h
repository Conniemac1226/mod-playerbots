#ifndef _PLAYERBOT_GRUULMULTIPLIERS_H
#define _PLAYERBOT_GRUULMULTIPLIERS_H

#include "Multiplier.h"

class GruulMultiplier : public Multiplier
{
public:
    GruulMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gruul") {}
    
    float GetValue(Action* action) override
    {
        // Prioritize certain actions during specific boss phases
        return 1.0f;
    }
};

class MaulgarAddMultiplier : public Multiplier
{
public:
    MaulgarAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "maulgar add") {}
    float GetValue(Action* action) override;
};

#endif