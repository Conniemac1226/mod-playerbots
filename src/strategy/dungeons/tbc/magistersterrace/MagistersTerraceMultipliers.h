#pragma once

#include "Multiplier.h"
#include "PlayerbotAI.h"

class MagistersTerraceMultiplier : public Multiplier
{
public:
    MagistersTerraceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magisters terrace") {}
    virtual float GetValue(Action* action) override;
};