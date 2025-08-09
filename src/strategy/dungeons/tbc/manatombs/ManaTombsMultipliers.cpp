#include "ManaTombsMultipliers.h"
#include "ManaTombsActions.h"

float ManaTombsMultiplier::GetValue(Action* action)
{
    // Following proven pattern of minimal multipliers to avoid priority violations
    // All priorities are set directly in strategy triggers
    return 1.0f;
}