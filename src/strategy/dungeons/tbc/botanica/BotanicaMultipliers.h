#ifndef _PLAYERBOT_BOTANICAMULTIPLIERS_H
#define _PLAYERBOT_BOTANICAMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class SarannisMultiplier : public Multiplier
{
public:
    SarannisMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "commander sarannis") {}
    float GetValue(Action* action) override;
};

class FreywinnMultiplier : public Multiplier
{
public:
    FreywinnMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high botanist freywinn") {}
    float GetValue(Action* action) override;
};

class LajMultiplier : public Multiplier
{
public:
    LajMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "laj") {}
    float GetValue(Action* action) override;
};

class ThorngrinMultiplier : public Multiplier
{
public:
    ThorngrinMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "thorngrin the tender") {}
    float GetValue(Action* action) override;
};

class WarpSplinterMultiplier : public Multiplier
{
public:
    WarpSplinterMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "warp splinter") {}
    float GetValue(Action* action) override;
};

class SarannisAddMultiplier : public Multiplier
{
public:
    SarannisAddMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sarannis add") {}
    float GetValue(Action* action) override;
};

class FreywinnAddMultiplier : public Multiplier
{
public:
    FreywinnAddMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "freywinn add") {}
    float GetValue(Action* action) override;
};

#endif