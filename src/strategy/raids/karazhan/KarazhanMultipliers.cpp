#include "KarazhanMultipliers.h"
#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "GenericActions.h"
#include "AttackAction.h"
#include "Playerbots.h"

float AttumenMultiplier::GetValue(Action* action)
{
    // Currently no specific multipliers needed
    // Can be added later if priority adjustments are needed
    return 1.0f;
}