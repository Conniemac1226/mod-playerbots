#ifndef _PLAYERBOT_RAIDZASTRATEGY_H
#define _PLAYERBOT_RAIDZASTRATEGY_H

#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Action.h"

// Base Zul'Aman strategy
class RaidZaStrategy : public Strategy
{
public:
    RaidZaStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "zulaman"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Nalorakk (Bear) Strategy
class RaidZaNalorakkStrategy : public Strategy
{
public:
    RaidZaNalorakkStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "nalorakk"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Akil'zon (Eagle) Strategy
class RaidZaAkilzonStrategy : public Strategy
{
public:
    RaidZaAkilzonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "akilzon"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Jan'alai (Dragonhawk) Strategy
class RaidZaJanalaiStrategy : public Strategy
{
public:
    RaidZaJanalaiStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "janalai"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Halazzi (Lynx) Strategy
class RaidZaHalazziStrategy : public Strategy
{
public:
    RaidZaHalazziStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "halazzi"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Hex Lord Malacrass Strategy
class RaidZaHexLordStrategy : public Strategy
{
public:
    RaidZaHexLordStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "hex lord malacrass"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// Zul'jin Strategy
class RaidZaZuljinStrategy : public Strategy
{
public:
    RaidZaZuljinStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "zuljin"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif