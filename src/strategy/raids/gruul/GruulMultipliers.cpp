#include "GruulMultipliers.h"
#include "GruulActions.h"
#include "ChooseTargetActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float MaulgarAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when council members are present - prevents boss/add oscillation
    // Following proven pattern from CLAUDE.md:678-702
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for council members using same pattern as trigger
    uint32 councilIds[] = {
        18832, // NPC_KROSH_FIREHAND
        18834, // NPC_OLM_THE_SUMMONER  
        18835, // NPC_KIGGLER_THE_CRAZED
        18836  // NPC_BLINDEYE_THE_SEER
    };
    
    bool councilPresent = false;
    for (uint32 npcId : councilIds)
    {
        Unit* council = bot->FindNearestCreature(npcId, 150.0f, true);
        if (council && council->IsAlive() && council->IsInCombat())
        {
            councilPresent = true;
            break;
        }
    }
    
    if (councilPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when council present
    }
    
    return 1.0f;
}