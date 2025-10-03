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

    bool isDpsAssist = dynamic_cast<DpsAssistAction*>(action) != nullptr;
    bool isTankAssist = dynamic_cast<TankAssistAction*>(action) != nullptr;

    if (!isDpsAssist && !isTankAssist)
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
        // Block DpsAssist for all, but allow TankAssist for tanks during Lady Vashj adds
        if (isDpsAssist)
            return 0.0f;
        if (isTankAssist && !botAI->IsTank(bot))
            return 0.0f;
    }

    if (blockRangedOnly && PlayerbotAI::IsRanged(bot))
    {
        // Block DpsAssist for ranged only
        if (isDpsAssist)
            return 0.0f;
    }

    return 1.0f;
}

float MorogrimOfftankMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI || !action)
        return 1.0f;

    // WotLK Pattern: Only affects assist tanks (off-tanks)
    if (!botAI->IsAssistTank(bot))
        return 1.0f;

    // Check if Morogrim Tidewalker encounter is active
    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // Check for presence of murloc adds that need off-tank pickup
    bool murlocsPresent = false;
    if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets"))
    {
        GuidVector const npcs = npcsValue->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_TIDEWALKER_LURKER)
            {
                murlocsPresent = true;
                break;
            }
        }
    }

    // WotLK Pattern: Block TankAssist when murlocs need off-tank pickup
    if (murlocsPresent && dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f; // Force off-tank to use MorogrimOfftankMurlocsAction instead
    }

    return 1.0f;
}

float VashjAddsMultiplier::GetValue(Action* action)
{
    if (!bot || !botAI || !action)
        return 1.0f;

    // Only affect DPS players during Lady Vashj encounter
    if (botAI->IsHeal(bot))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    auto getAddPriority = [&](uint32 entry) -> uint8
    {
        // CRITICAL: Tainted Elementals are ALWAYS highest priority - they drop cores needed to exit Phase 2
        // They despawn in 15 seconds and spawn 60-100 yards away, so must be killed IMMEDIATELY
        if (entry == NPC_TAINTED_ELEMENTAL)
            return 5; // Highest priority for all roles

        if (PlayerbotAI::IsRangedDps(bot))
        {
            switch (entry)
            {
                case NPC_COILFANG_STRIDER: return 4;       // Ranged kite + burst target
                case NPC_ENCHANTED_ELEMENTAL: return 2;    // Clean up remaining adds
                case NPC_COILFANG_ELITE: return 1;         // Assist only when nothing else alive
                default: return 0;
            }
        }

        // Melee and tanks focus on elites first, then elementals
        switch (entry)
        {
            case NPC_COILFANG_ELITE: return 4;             // Maintain elite control
            case NPC_ENCHANTED_ELEMENTAL: return 2;        // Clean up ambient elementals
            case NPC_COILFANG_STRIDER: return 1;           // Low priority for melee (avoid fear)
            default: return 0;
        }
    };

    auto actionNameToEntry = [](std::string const& actionName) -> uint32
    {
        if (actionName == "vashj tainted elemental" || actionName == "vashj tainted core")
            return NPC_TAINTED_ELEMENTAL;

        if (actionName == "vashj coilfang strider")
            return NPC_COILFANG_STRIDER;

        if (actionName == "vashj coilfang elite")
            return NPC_COILFANG_ELITE;

        if (actionName == "vashj enchanted elemental")
            return NPC_ENCHANTED_ELEMENTAL;

        return 0;
    };

    // Phase 2 - Lady Vashj is IMMUNE during magic barrier
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        // BLOCK BOSS TARGETING - she's immune!
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f; // Force DPS to target adds only
        }

        // Allow TankAssistAction only for main tanks (they might need to stay on boss for positioning)
        // Off-tanks should use specific add targeting actions instead
        if (dynamic_cast<TankAssistAction*>(action) && botAI->IsTank(bot) && !botAI->IsMainTank(bot))
        {
            return 0.0f; // Force off-tanks to use VashjOfftankAddsAction
        }

        Unit* currentTarget = AI_VALUE(Unit*, "current target");

        // WotLK Anti-Ping-Pong Pattern with priority override: stay on higher or equal priority targets
        if (currentTarget && currentTarget->IsAlive())
        {
            uint32 currentEntry = currentTarget->GetEntry();
            uint8 currentPriority = getAddPriority(currentEntry);
            if (currentPriority > 0)
            {
                std::string const actionName = action->getName();
                uint32 actionEntry = actionNameToEntry(actionName);

                if (actionEntry)
                {
                    uint8 actionPriority = getAddPriority(actionEntry);

                    // Allow switching to same, higher or unknown priority targets
                    if (actionEntry != currentEntry && actionPriority > 0 && actionPriority < currentPriority)
                    {
                        return 0.0f;
                    }
                }
            }
        }
    }

    return 1.0f;
}
