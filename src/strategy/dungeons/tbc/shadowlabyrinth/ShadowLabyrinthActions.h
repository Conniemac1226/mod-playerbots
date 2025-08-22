#ifndef _PLAYERBOT_SHADOWLABYRINTHACTIONS_H
#define _PLAYERBOT_SHADOWLABYRINTHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const uint32 SPELL_CORROSIVE_ACID = 33551;
const uint32 SL_SPELL_FEAR = 33547;

const uint32 SPELL_INCITE_CHAOS = 33676;
const uint32 SPELL_INCITE_CHAOS_B = 33684;
const uint32 SL_SPELL_CHARGE = 33709;
const uint32 SL_SPELL_WAR_STOMP = 33707;

const uint32 SPELL_RAIN_OF_FIRE = 33617;
const uint32 SPELL_DRAW_SHADOWS = 33563;
const uint32 SPELL_SHADOWBOLT_VOLLEY = 33841;
const uint32 SPELL_BANISH = 38791;

const uint32 SPELL_SONIC_BOOM_CAST = 33923;
const uint32 SPELL_SONIC_BOOM_EFFECT = 38795;
const uint32 SPELL_MURMURS_TOUCH = 33711;
const uint32 SPELL_RESONANCE = 33657;
const uint32 SL_SPELL_MAGNETIC_PULL = 33689;
const uint32 SPELL_THUNDERING_STORM = 39365;

const uint32 NPC_VOID_TRAVELER = 19226;
const uint32 NPC_INCITE_TRIGGER = 19300;

class AvoidCorrosiveAcidAction : public MovementAction
{
public:
    AvoidCorrosiveAcidAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid corrosive acid") {}
    bool Execute(Event event) override;
};

class HellmawFearReactAction : public MovementAction
{
public:
    HellmawFearReactAction(PlayerbotAI* botAI) : MovementAction(botAI, "hellmaw fear react") {}
    bool Execute(Event event) override;
};

class InciteChaosReactAction : public MovementAction
{
public:
    InciteChaosReactAction(PlayerbotAI* botAI) : MovementAction(botAI, "incite chaos react") {}
    bool Execute(Event event) override;
};

class AvoidWarStompAction : public MovementAction
{
public:
    AvoidWarStompAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid war stomp") {}
    bool Execute(Event event) override;
};

class BlackheartChargeReactAction : public MovementAction
{
public:
    BlackheartChargeReactAction(PlayerbotAI* botAI) : MovementAction(botAI, "blackheart charge react") {}
    bool Execute(Event event) override;
};

class VoidTravelerPriorityAction : public AttackAction
{
public:
    VoidTravelerPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, "void traveler priority") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class MoveFromRainOfFireAction : public MovementAction
{
public:
    MoveFromRainOfFireAction(PlayerbotAI* botAI) : MovementAction(botAI, "move from rain of fire") {}
    bool Execute(Event event) override;
};

class DrawShadowsReactAction : public MovementAction
{
public:
    DrawShadowsReactAction(PlayerbotAI* botAI) : MovementAction(botAI, "draw shadows react") {}
    bool Execute(Event event) override;
};

class MurmurSonicBoomAction : public MovementAction
{
public:
    MurmurSonicBoomAction(PlayerbotAI* botAI) : MovementAction(botAI, "murmur sonic boom") {}
    bool Execute(Event event) override;
};

class MurmurResonanceAction : public MovementAction
{
public:
    MurmurResonanceAction(PlayerbotAI* botAI) : MovementAction(botAI, "murmur resonance") {}
    bool Execute(Event event) override;
};

class MurmurMagneticPullAction : public MovementAction
{
public:
    MurmurMagneticPullAction(PlayerbotAI* botAI) : MovementAction(botAI, "murmur magnetic pull") {}
    bool Execute(Event event) override;
};

class MurmurThunderingStormAction : public MovementAction
{
public:
    MurmurThunderingStormAction(PlayerbotAI* botAI) : MovementAction(botAI, "murmur thundering storm") {}
    bool Execute(Event event) override;
};

#endif