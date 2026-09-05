#include "BlackTempleMultipliers.h"
#include "BlackTempleActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "Unit.h"
#include "Value.h"
#include "AiObjectContext.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#define SPELL_IMPALING_SPINE 39837

float NajentusTargetMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    
    if (!boss)
        return 1.0f;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // If we're attacking something else when Naj'entus is active, prioritize boss
    if (currentTarget && currentTarget != boss && boss->IsAlive())
    {
        if (dynamic_cast<DpsAssistAction*>(action) || 
            dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f; // Don't assist on other targets
        }
    }

    return 1.0f;
}

float NajentusImpaledMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI)
        return 1.0f;

    // Check if we're impaled
    if (bot->HasAura(SPELL_IMPALING_SPINE))
    {
        // Can't move when impaled
        if (dynamic_cast<MovementAction*>(action))
            return 0.0f;
            
        // Can still cast
        return 1.0f;
    }

    // Check for nearby impaled allies
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return 1.0f;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            float distance = bot->GetDistance(member);
            if (distance < 10.0f)
            {
                // Increase priority of movement away from impaled targets
                if (dynamic_cast<NajentusAvoidImpaledTargetAction*>(action))
                    return 2.0f;
            }
        }
    }

    return 1.0f;
}
