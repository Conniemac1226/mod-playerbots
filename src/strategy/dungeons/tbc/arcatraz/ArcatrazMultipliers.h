#ifndef _PLAYERBOT_ARCATRAZMULTIPLIERS_H
#define _PLAYERBOT_ARCATRAZMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class ZerekethMultiplier : public Multiplier
{
public:
    ZerekethMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "zereketh the unbound") {}
    float GetValue(Action* action) override;
};

class DalliahMultiplier : public Multiplier
{
public:
    DalliahMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "dalliah the doomsayer") {}
    float GetValue(Action* action) override;
};

class SoccothratesMultiplier : public Multiplier
{
public:
    SoccothratesMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "wrath-scryer soccothrates") {}
    float GetValue(Action* action) override;
};

class SkyrissMultiplier : public Multiplier
{
public:
    SkyrissMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "harbinger skyriss") {}
    float GetValue(Action* action) override;
};

class MellicharAddMultiplier : public Multiplier
{
public:
    MellicharAddMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mellichar add") {}
    float GetValue(Action* action) override;
};

#endif