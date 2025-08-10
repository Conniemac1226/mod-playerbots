#ifndef _PLAYERBOT_SHATTEREDHALLSACTIONS_H
#define _PLAYERBOT_SHATTEREDHALLSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const uint32 SPELL_DEATH_COIL = 30500;
const uint32 SPELL_DARK_SPIN = 30502;
const uint32 SPELL_SHADOW_FISSURE = 30496;
const uint32 SPELL_SHADOW_CLEAVE = 30495;
const uint32 SPELL_LESSER_SHADOW_FISSURE = 30744;

const uint32 SPELL_BLAST_WAVE = 30600;
const uint32 SPELL_FEAR = 30584;
const uint32 SPELL_THUNDERCLAP = 30633;
const uint32 SPELL_BURNING_MAUL = 30598;

const uint32 SPELL_BLADE_DANCE_DMG = 30739;

const uint32 NPC_SHATTERED_ASSASSIN = 17695;
const uint32 NPC_PEON = 17083;

class AvoidShadowFissureAction : public MovementAction
{
public:
    AvoidShadowFissureAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid shadow fissure") {}
    bool Execute(Event event) override;
};

class AvoidDarkSpinAction : public MovementAction
{
public:
    AvoidDarkSpinAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid dark spin") {}
    bool Execute(Event event) override;
};

class MoveFromBlastWaveAction : public MovementAction
{
public:
    MoveFromBlastWaveAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from blast wave") {}
    bool Execute(Event event) override;
};

class AvoidBurningMaulAction : public MovementAction
{
public:
    AvoidBurningMaulAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid burning maul") {}
    bool Execute(Event event) override;
};

class AvoidBladeDanceAction : public MovementAction
{
public:
    AvoidBladeDanceAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid blade dance") {}
    bool Execute(Event event) override;
};

class KillShatteredAssassinsAction : public AttackAction
{
public:
    KillShatteredAssassinsAction(PlayerbotAI* botAI) : AttackAction(botAI, "kill shattered assassins") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class NetheKursePeonPriorityAction : public AttackAction
{
public:
    NetheKursePeonPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, "nethekurse peon priority") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif