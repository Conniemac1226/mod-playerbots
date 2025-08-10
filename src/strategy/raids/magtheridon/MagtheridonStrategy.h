#ifndef _PLAYERBOT_MAGTHERIDONSTRATEGY_H
#define _PLAYERBOT_MAGTHERIDONSTRATEGY_H

#include "Strategy.h"
#include "AiObjectContext.h"

// Base Magtheridon strategy
class MagtheridonStrategy : public Strategy
{
public:
    MagtheridonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "magtheridon"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Phase 1: Channelers alive
class MagtheridonChannelersStrategy : public Strategy
{
public:
    MagtheridonChannelersStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "magtheridon channelers"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

// Phase 2: Magtheridon released
class MagtheridonReleasedStrategy : public Strategy
{
public:
    MagtheridonReleasedStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "magtheridon released"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

// Phase 3: Cave in (30% health)
class MagtheridonCaveInStrategy : public Strategy
{
public:
    MagtheridonCaveInStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "magtheridon cave in"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif