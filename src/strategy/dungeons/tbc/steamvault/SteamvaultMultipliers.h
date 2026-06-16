#pragma once

#include "Multiplier.h"
#include "PlayerbotAI.h"

class SteamvaultMultiplier : public Multiplier
{
public:
    SteamvaultMultiplier(PlayerbotAI* ai) : Multiplier(ai, "steamvault") {}
    virtual float GetValue(Action* action) override;
};

class ThespiaWaterElementalMultiplier : public Multiplier
{
public:
    ThespiaWaterElementalMultiplier(PlayerbotAI* ai) : Multiplier(ai, "thespia water elemental") {}
    float GetValue(Action* action) override;
};

class SteamriggerMechanicMultiplier : public Multiplier
{
public:
    SteamriggerMechanicMultiplier(PlayerbotAI* ai) : Multiplier(ai, "steamrigger mechanic") {}
    float GetValue(Action* action) override;
};

class KalithreshSpellReflectionMultiplier : public Multiplier
{
public:
    KalithreshSpellReflectionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kalithresh spell reflection") {}
    float GetValue(Action* action) override;
};

class KalithreshDistillerMultiplier : public Multiplier
{
public:
    KalithreshDistillerMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kalithresh distiller") {}
    float GetValue(Action* action) override;
};
