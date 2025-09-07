#ifndef _PLAYERBOT_KARAZHANMULTIPLIERS_H
#define _PLAYERBOT_KARAZHANMULTIPLIERS_H

#include "Multiplier.h"

class AttumenMultiplier : public Multiplier
{
public:
    AttumenMultiplier(PlayerbotAI* ai) : Multiplier(ai, "attumen") {}
    float GetValue(Action* action) override;
};

class CuratorAddMultiplier : public Multiplier
{
public:
    CuratorAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "curator add") {}
    float GetValue(Action* action) override;
};

class IllhoofAddMultiplier : public Multiplier
{
public:
    IllhoofAddMultiplier(PlayerbotAI* ai) : Multiplier(ai, "illhoof add") {}
    float GetValue(Action* action) override;
};

#endif