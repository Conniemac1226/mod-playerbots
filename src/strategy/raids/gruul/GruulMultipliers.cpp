#include "GruulMultipliers.h"
#include "GruulActions.h"
#include "ChooseTargetActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float MaulgarAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when council members OR spawned adds are present
    // Following proven pattern from CLAUDE.md:753-778
    if (botAI->IsHeal(bot) || botAI->IsTank(bot)) { return 1.0f; }
    
    // Check for council members using same pattern as trigger
    uint32 councilIds[] = {
        18832, // NPC_KROSH_FIREHAND
        18834, // NPC_OLM_THE_SUMMONER  
        18835, // NPC_KIGGLER_THE_CRAZED
        18836  // NPC_BLINDEYE_THE_SEER
    };
    
    bool addPresent = false;
    for (uint32 npcId : councilIds)
    {
        Unit* council = bot->FindNearestCreature(npcId, 150.0f, true);
        if (council && council->IsAlive() && council->IsInCombat())
        {
            addPresent = true;
            break;
        }
    }
    
    // ALSO check for spawned Wild Fel Stalkers (critical for proper targeting)
    if (!addPresent)
    {
        const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        for (const auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->IsInCombat() && unit->GetEntry() == 18847) // NPC_WILD_FEL_STALKER
            {
                addPresent = true;
                break;
            }
        }
    }
    
    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when any adds present
    }
    
    return 1.0f;
}
