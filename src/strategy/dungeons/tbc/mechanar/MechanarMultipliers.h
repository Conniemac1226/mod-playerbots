#pragma once
#include "Multiplier.h"

class MechanarMultiplier : public Multiplier
{
public:
    MechanarMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mechanar") {}
    float GetValue(Action* action) override;
};