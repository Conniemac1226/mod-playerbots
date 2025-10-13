#include "SethekkHallsMultipliers.h"
#include "ChooseTargetActions.h"
#include "SethekkHallsTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// REMOVED: CharmingTotemMultiplier
// ICC Pattern: Skull marking naturally handles target priority
// No need to block DpsAssist - bots will target skull-marked totem automatically

float BroodOfAnzuMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Brood of Anzu are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }

    // ICC Pattern (RaidIccTriggers.cpp:301-312): No IsInCombat check for spawned adds
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool broodPresent = false;

    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_BROOD_OF_ANZU)
            {
                broodPresent = true;
                break;
            }
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