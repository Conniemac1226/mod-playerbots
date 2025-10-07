#include "SethekkHallsMultipliers.h"
#include "ChooseTargetActions.h"
#include "SethekkHallsTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float CharmingTotemMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Charming Totems are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }

    // ICC Pattern (RaidIccTriggers.cpp:301-312): No IsInCombat check for spawned adds
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool totemPresent = false;

    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_CHARMING_TOTEM)
            {
                totemPresent = true;
                break;
            }
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