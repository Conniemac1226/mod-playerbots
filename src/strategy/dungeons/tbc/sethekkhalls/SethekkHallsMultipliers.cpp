#include "SethekkHallsMultipliers.h"
#include "ChooseTargetActions.h"
#include "SethekkHallsTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float CharmingTotemMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Charming Totems are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Charming Totems using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool totemPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_CHARMING_TOTEM)
        {
            totemPresent = true;
            break;
        }
    }
    
    if (totemPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when totems present
    }
    
    return 1.0f;
}

float BroodOfAnzuMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Brood of Anzu are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Brood of Anzu using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool broodPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_BROOD_OF_ANZU)
        {
            broodPresent = true;
            break;
        }
    }
    
    if (broodPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when brood present
    }
    
    return 1.0f;
}

float IkissPhaseMultiplier::GetValue(Action* action)
{
    return 1.0f;
}