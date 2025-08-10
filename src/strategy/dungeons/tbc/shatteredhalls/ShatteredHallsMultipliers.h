#ifndef _PLAYERBOT_SHATTEREDHALLSMULTIPLIERS_H
#define _PLAYERBOT_SHATTEREDHALLSMULTIPLIERS_H

#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class NethekurseMultiplier : public Multiplier
{
public:
    NethekurseMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "grand warlock nethekurse") {}
    float GetValue(Action* action) override;
};

class OmroggMultiplier : public Multiplier
{
public:
    OmroggMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "warbringer o'mrogg") {}
    float GetValue(Action* action) override;
};

class KargathMultiplier : public Multiplier
{
public:
    KargathMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "warchief kargath bladefist") {}
    float GetValue(Action* action) override;
};

#endif