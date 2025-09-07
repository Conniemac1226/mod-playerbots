#include "ManaTombsMultipliers.h"
#include "ManaTombsActions.h"
#include "ChooseTargetActions.h"
#include "ManaTombsTriggers.h"

float ManaTombsMultiplier::GetValue(Action* action)
{
    // Following proven pattern of minimal multipliers to avoid priority violations
    // All priorities are set directly in strategy triggers
    return 1.0f;
}

float EtherealBeaconMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Ethereal Beacons are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Ethereal Beacons using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool beaconPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_ETHEREAL_BEACON)
        {
            beaconPresent = true;
            break;
        }
    }
    
    if (beaconPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when beacons present
    }
    
    return 1.0f;
}