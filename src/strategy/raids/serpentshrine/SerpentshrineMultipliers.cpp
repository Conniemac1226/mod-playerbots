#include "SerpentshrineMultipliers.h"
#include "SerpentshrineActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Value.h"
#include "ChooseTargetActions.h"

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

    bool const isKillAddsAction = action && action->getName() == "hydross kill adds";

    if (addCount >= 4)
    {
        return isKillAddsAction ? 1.4f : 1.0f;
    }
    else if (addCount >= 2)
    {
        return isKillAddsAction ? 1.2f : 1.0f;
    }

    return isKillAddsAction ? 1.05f : 1.0f;
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

float SerpentshrinePriorityMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI || !action)
    {
        return 1.0f;
    }

    bool isAssist = dynamic_cast<DpsAssistAction*>(action) != nullptr ||
                    dynamic_cast<TankAssistAction*>(action) != nullptr;
    if (!isAssist)
    {
        return 1.0f;
    }

    bool blockAll = false;
    bool blockRangedOnly = false;

    if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
    {
        GuidVector const npcs = npcsValue->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || !unit->IsInCombat())
                continue;

            switch (unit->GetEntry())
            {
                case NPC_CYCLONE_KARATHRESS:
                case NPC_FATHOM_GUARD_SHARKKIS:
                case NPC_FATHOM_GUARD_TIDALVESS:
                case NPC_FATHOM_GUARD_CARIBDIS:
                case NPC_SPITFIRE_TOTEM:
                case NPC_GREATER_EARTHBIND_TOTEM:
                case NPC_GREATER_POISON_CLEANSING_TOTEM:
                case NPC_COILFANG_STRIDER:
                case NPC_COILFANG_ELITE:
                case NPC_ENCHANTED_ELEMENTAL:
                case NPC_TIDEWALKER_LURKER:
                case NPC_WATER_GLOBULE:
                    blockAll = true;
                    break;
                case NPC_SHADOW_OF_LEOTHERAS:
                case NPC_TAINTED_ELEMENTAL:
                    blockRangedOnly = true;
                    break;
                default:
                    break;
            }

            if (blockAll)
                break;
        }
    }

    if (blockAll)
    {
        return 0.0f;
    }

    if (blockRangedOnly && PlayerbotAI::IsRanged(bot))
    {
        return 0.0f;
    }

    return 1.0f;
}
