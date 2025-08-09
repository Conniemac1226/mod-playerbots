#ifndef _PLAYERBOT_KARAZHANACTIONS_H
#define _PLAYERBOT_KARAZHANACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Attumen the Huntsman
class AttumenAvoidChargeAction : public MovementAction
{
public:
    AttumenAvoidChargeAction(PlayerbotAI* ai) : MovementAction(ai, "attumen avoid charge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttumenPositionAction : public MovementAction
{
public:
    AttumenPositionAction(PlayerbotAI* ai) : MovementAction(ai, "attumen position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif