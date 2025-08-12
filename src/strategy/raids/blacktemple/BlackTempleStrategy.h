#ifndef _PLAYERBOT_RAIDBTSTRATEGY_H
#define _PLAYERBOT_RAIDBTSTRATEGY_H

#include "Multiplier.h"
#include "Strategy.h"

class RaidBtStrategy : public Strategy
{
public:
    RaidBtStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "black temple"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtNajentusStrategy : public Strategy
{
public:
    RaidBtNajentusStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "najentus"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtSupremusStrategy : public Strategy
{
public:
    RaidBtSupremusStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "supremus"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtShadeOfAkamaStrategy : public Strategy
{
public:
    RaidBtShadeOfAkamaStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "shade of akama"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtTeronGorefiendStrategy : public Strategy
{
public:
    RaidBtTeronGorefiendStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "teron gorefiend"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtGurtoggBloodboilStrategy : public Strategy
{
public:
    RaidBtGurtoggBloodboilStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "gurtogg bloodboil"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtReliquaryOfSoulsStrategy : public Strategy
{
public:
    RaidBtReliquaryOfSoulsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "reliquary of souls"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

class RaidBtMotherShahrazStrategy : public Strategy
{
public:
    RaidBtMotherShahrazStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "mother shahraz"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif