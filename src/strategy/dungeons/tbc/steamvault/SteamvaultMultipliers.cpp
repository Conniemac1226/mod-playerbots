#include "SteamvaultMultipliers.h"
#include "SteamvaultTriggers.h"
#include "Unit.h"

float SteamvaultMultiplier::GetValue(Action* action)
{
    // No multipliers needed - using proper priority values in strategy
    // Following the pattern from fixed Sethekk Halls, Auchenai Crypts, and Magisters' Terrace
    return 1.0f;
}