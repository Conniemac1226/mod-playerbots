#include "SethekkHallsMultipliers.h"
#include "SethekkHallsActions.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Playerbots.h"

float CharmingTotemMultiplier::GetValue(Action* action)
{
    // DEPRECATED: Multipliers removed to prevent priority stacking
    // The strategy now uses higher base priorities instead
    // Keeping minimal implementation for compatibility
    return 1.0f;
}

float IkissPhaseMultiplier::GetValue(Action* action)
{
    // DEPRECATED: Multipliers removed to prevent priority stacking
    // The strategy now uses higher base priorities instead
    // Keeping minimal implementation for compatibility
    return 1.0f;
}