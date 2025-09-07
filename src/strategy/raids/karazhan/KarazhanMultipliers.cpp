#include "KarazhanMultipliers.h"
#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "ChooseTargetActions.h"
#include "GenericActions.h"
#include "AttackAction.h"
#include "Playerbots.h"

float AttumenMultiplier::GetValue(Action* action)
{
    // Currently no specific multipliers needed
    // Can be added later if priority adjustments are needed
    return 1.0f;
}

float CuratorAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Astral Flares are present - prevents boss/add oscillation
    // Following proven pattern from UnderbogMultipliers.cpp:13-38
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Astral Flares using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool flarePresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_ASTRAL_FLARE)
        {
            flarePresent = true;
            break;
        }
    }
    
    if (flarePresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when flares present
    }
    
    return 1.0f;
}

float IllhoofAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Illhoof adds are present - prevents boss/add oscillation
    // Following proven pattern from UnderbogMultipliers.cpp:13-38
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Illhoof adds using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool addPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            uint32 entry = unit->GetEntry();
            // Priority: Kilrek > Demon Chains > Fiendish Imps
            if (entry == NPC_KILTREK || entry == NPC_DEMON_CHAINS || entry == NPC_FIENDISH_IMP)
            {
                addPresent = true;
                break;
            }
        }
    }
    
    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when adds present
    }
    
    return 1.0f;
}

float MalchezaarAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Netherspite Infernals are present - prevents boss/add oscillation
    // Following proven pattern from CLAUDE.md:678-702
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for spawned infernals using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool infernalPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
        {
            infernalPresent = true;
            break;
        }
    }
    
    if (infernalPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when infernals present
    }
    
    return 1.0f;
}

float NightbaneAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Restless Skeletons are present - prevents boss/add oscillation
    // Following proven pattern from CLAUDE.md:678-702
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for spawned skeletons using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool skeletonPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_RESTLESS_SKELETON)
        {
            skeletonPresent = true;
            break;
        }
    }
    
    if (skeletonPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when skeletons present
    }
    
    return 1.0f;
}