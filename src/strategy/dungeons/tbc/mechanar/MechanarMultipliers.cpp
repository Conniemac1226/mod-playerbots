#include "MechanarMultipliers.h"
#include "MechanarActions.h"
#include "PlayerbotAI.h"

float MechanarMultiplier::GetValue(Action* action)
{
    // No multipliers needed - using safe priority values based on lessons learned
    // All priorities are handled directly in strategy with ACTION_* constants
    return 1.0f;
}