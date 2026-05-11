#ifndef _PLAYERBOT_SHADOWLABYRINTHACTIONS_H
#define _PLAYERBOT_SHADOWLABYRINTHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ShadowLabyrinthHelpers.h"

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

class VorpilSpreadAction : public MovementAction
{
public:
    VorpilSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "vorpil spread") {}
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
