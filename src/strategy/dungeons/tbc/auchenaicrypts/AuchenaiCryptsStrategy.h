#ifndef _PLAYERBOT_TBCDUNGEONSACSTRATEGY_H
#define _PLAYERBOT_TBCDUNGEONSACSTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"
#include "AuchenaiCryptsTriggers.h"
#include "AuchenaiCryptsActionContext.h"
#include "AuchenaiCryptsTriggerContext.h"

class TbcDungeonACStrategy : public Strategy
{
public:
    TbcDungeonACStrategy(PlayerbotAI* ai) : Strategy(ai) {}

public:
    void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
    std::string const getName() override { return "auchenai crypts"; }
};

#endif