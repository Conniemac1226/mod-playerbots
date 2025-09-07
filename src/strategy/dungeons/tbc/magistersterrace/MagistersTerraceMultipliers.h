#pragma once

#include "Multiplier.h"
#include "PlayerbotAI.h"

class MagistersTerraceMultiplier : public Multiplier
{
public:
    MagistersTerraceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "magisters terrace") {}
    virtual float GetValue(Action* action) override;
};

class VexallusPureEnergyMultiplier : public Multiplier
{
public:
    VexallusPureEnergyMultiplier(PlayerbotAI* ai) : Multiplier(ai, "vexallus pure energy") {}
    float GetValue(Action* action) override;
};

class SelinFelCrystalMultiplier : public Multiplier
{
public:
    SelinFelCrystalMultiplier(PlayerbotAI* ai) : Multiplier(ai, "selin fel crystal") {}
    float GetValue(Action* action) override;
};

class DelrissaAddMultiplier : public Multiplier
{
public:
    DelrissaAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "delrissa add") {}
    float GetValue(Action* action) override;
};