#include "UnderbogMultipliers.h"
#include "UnderbogActions.h"

float UnderbogMultiplier::GetValue(Action* action)
{
    // Following proven pattern of minimal multipliers to avoid priority violations
    // All priorities are set directly in strategy triggers
    return 1.0f;
}