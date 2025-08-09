#pragma once
#include "MovementActions.h"

// Base class for Black Morass movement actions
class BlackMorassMovementAction : public MovementAction
{
public:
    BlackMorassMovementAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}
};