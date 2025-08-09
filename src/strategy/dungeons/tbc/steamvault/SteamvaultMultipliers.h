#pragma once

#include "Multiplier.h"
#include "PlayerbotAI.h"

class SteamvaultMultiplier : public Multiplier
{
public:
    SteamvaultMultiplier(PlayerbotAI* ai) : Multiplier(ai, "steamvault") {}
    virtual float GetValue(Action* action) override;
};