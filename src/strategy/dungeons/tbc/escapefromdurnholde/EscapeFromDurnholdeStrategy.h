#pragma once
#include "Strategy.h"
#include "EscapeFromDurnholdeTriggerContext.h"
#include "EscapeFromDurnholdeActionContext.h"

class EscapeFromDurnholdeStrategy : public Strategy
{
public:
    EscapeFromDurnholdeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers);
    virtual std::string const getName() { return "escape from durnholde"; }
    virtual uint32 GetType() const { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_TANK | STRATEGY_TYPE_HEAL; }
};