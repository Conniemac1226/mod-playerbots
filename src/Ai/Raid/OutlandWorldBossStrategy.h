/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OUTLANDWORLDBOSSSTRATEGY_H
#define _PLAYERBOT_OUTLANDWORLDBOSSSTRATEGY_H

#include "MovementActions.h"
#include "Strategy.h"

enum OutlandWorldBossSpells
{
    SPELL_DOOMWALKER_EARTHQUAKE = 32686,
    SPELL_DOOMWALKER_CHAIN_LIGHTNING = 33665,
    SPELL_DOOM_LORD_KAZZAK_MARK_OF_KAZZAK = 32960,
    SPELL_DOOM_LORD_KAZZAK_TWISTED_REFLECTION = 21063
};

class DoomwalkerChainLightningSpreadAction : public MovementAction
{
public:
    DoomwalkerChainLightningSpreadAction(PlayerbotAI* botAI,
                                         std::string const name = "doomwalker spread for chain lightning")
        : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DoomwalkerEarthquakeMoveAwayAction : public MovementAction
{
public:
    DoomwalkerEarthquakeMoveAwayAction(PlayerbotAI* botAI,
                                       std::string const name = "doomwalker move away from earthquake")
        : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DoomLordKazzakMoveAwayFromMarkAction : public MovementAction
{
public:
    DoomLordKazzakMoveAwayFromMarkAction(PlayerbotAI* botAI,
                                         std::string const name = "doom lord kazzak move away from mark of kazzak")
        : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DoomLordKazzakMoveAwayDuringTwistedReflectionAction : public MovementAction
{
public:
    DoomLordKazzakMoveAwayDuringTwistedReflectionAction(
        PlayerbotAI* botAI,
        std::string const name = "doom lord kazzak move away during twisted reflection")
        : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DoomwalkerChainLightningTrigger : public Trigger
{
public:
    DoomwalkerChainLightningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "doomwalker chain lightning") {}

    bool IsActive() override;
};

class DoomwalkerEarthquakeTrigger : public Trigger
{
public:
    DoomwalkerEarthquakeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "doomwalker earthquake") {}

    bool IsActive() override;
};

class DoomLordKazzakMarkOfKazzakTrigger : public Trigger
{
public:
    DoomLordKazzakMarkOfKazzakTrigger(PlayerbotAI* botAI) : Trigger(botAI, "doom lord kazzak mark of kazzak") {}

    bool IsActive() override;
};

class DoomLordKazzakTwistedReflectionTrigger : public Trigger
{
public:
    DoomLordKazzakTwistedReflectionTrigger(PlayerbotAI* botAI) :
        Trigger(botAI, "doom lord kazzak twisted reflection") {}

    bool IsActive() override;
};

class DoomLordKazzakMarkOfKazzakMultiplier : public Multiplier
{
public:
    DoomLordKazzakMarkOfKazzakMultiplier(PlayerbotAI* botAI) :
        Multiplier(botAI, "doom lord kazzak mark of kazzak") {}

    float GetValue(Action* action) override;
};

class DoomLordKazzakTwistedReflectionMultiplier : public Multiplier
{
public:
    DoomLordKazzakTwistedReflectionMultiplier(PlayerbotAI* botAI) :
        Multiplier(botAI, "doom lord kazzak twisted reflection") {}

    float GetValue(Action* action) override;
};

class RaidDoomwalkerStrategy : public Strategy
{
public:
    RaidDoomwalkerStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "doomwalker"; }

    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class RaidDoomLordKazzakStrategy : public Strategy
{
public:
    RaidDoomLordKazzakStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "doom lord kazzak"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
