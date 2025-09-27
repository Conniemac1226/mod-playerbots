#include "SerpentshrineMultipliers.h"
#include "SerpentshrineActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Value.h"

float HydrossResistanceMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI)
    {
        return 1.0f;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    
    if (hasCorruption && botAI->IsTank(bot))
    {
        uint32 natureResistance = bot->GetResistance(SPELL_SCHOOL_NATURE);
        if (natureResistance < 250)
        {
            return 0.8f;
        }
    }
    else if (!hasCorruption && botAI->IsTank(bot))
    {
        uint32 frostResistance = bot->GetResistance(SPELL_SCHOOL_FROST);
        if (frostResistance < 250)
        {
            return 0.8f;
        }
    }

    return 1.0f;
}

float HydrossAddsMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI)
    {
        return 1.0f;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return 1.0f;
    }
    GuidVector npcs = npcsValue->Get();
    
    uint32 addCount = 0;
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_SPAWN_OF_HYDROSS || 
            unit->GetEntry() == NPC_TAINTED_SPAWN_OF_HYDROSS)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 40.0f)
            {
                addCount++;
            }
        }
    }

    if (addCount >= 4)
    {
        return 0.7f;
    }
    else if (addCount >= 2)
    {
        return 0.85f;
    }

    return 1.0f;
}

float HydrossTankMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
    {
        return 1.0f;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget == boss)
    {
        return 0.9f;
    }

    return 1.0f;
}

float LeotherasThreatHoldMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI)
    {
        return 1.0f;
    }

    if (botAI->IsTank(bot))
    {
        return 1.0f;
    }

    uint32 holdUntil = AI_VALUE(uint32, "leotheras whirlwind hold until");
    if (holdUntil > getMSTime())
    {
        return 0.0f;
    }

    return 1.0f;
}
