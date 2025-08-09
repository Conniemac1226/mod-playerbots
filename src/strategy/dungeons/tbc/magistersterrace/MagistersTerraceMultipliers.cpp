#include "MagistersTerraceMultipliers.h"
#include "MagistersTerraceTriggers.h"
#include "Unit.h"

float MagistersTerraceMultiplier::GetValue(Action* action)
{
    // No multipliers needed - using proper priority values in strategy
    // Following the pattern from fixed Sethekk Halls and Auchenai Crypts
    return 1.0f;
}