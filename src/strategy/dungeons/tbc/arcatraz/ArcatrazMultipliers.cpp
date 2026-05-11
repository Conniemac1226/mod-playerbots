#include "Playerbots.h"
#include "ArcatrazMultipliers.h"
#include "ArcatrazActions.h"
#include "SharedDefines.h"

float ZerekethMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    std::string actionName = action->getName();
    
    // EMERGENCY: Boost void zone avoidance when in danger
    if (actionName == "avoid void zone")
    {
        Player* bot = botAI->GetBot();
        if (!bot) return 1.0f;
        
        if (bot->GetHealthPct() < 80.0f) // Taking damage from void zones
        {
            return 3.0f; // Critical priority when low health
        }
        return 2.0f; // Always elevated during encounter
    }
    
    // EMERGENCY: Boost Shadow Nova avoidance during cast
    if (actionName == "avoid shadow nova")
    {
        if (boss->FindCurrentSpellBySpellId(ARC_SPELL_SHADOW_NOVA))
        {
            return 4.0f; // Critical priority during cast
        }
        return 1.5f; // Elevated when boss is alive
    }
    
    // BOOST: Prioritize seed dispel when afflicted
    if (actionName == "seed of corruption dispel")
    {
        Player* bot = botAI->GetBot();
        if (bot && bot->HasAura(SPELL_SEED_OF_CORRUPTION))
        {
            return 2.5f; // High priority when afflicted
        }
    }
    
    return 1.0f;
}

float DalliahMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    std::string actionName = action->getName();
    
    // CRITICAL: Interrupt heal - highest priority
    if (actionName == "dalliah heal interrupt")
    {
        if (boss->FindCurrentSpellBySpellId(ARC_SPELL_HEAL))
        {
            return 5.0f; // Critical - must interrupt heal immediately
        }
        return 1.2f; // Slightly elevated when boss is alive
    }
    
    // EMERGENCY: Whirlwind avoidance
    if (actionName == "dalliah whirlwind")
    {
        if (boss->HasAura(ARC_SPELL_WHIRLWIND))
        {
            return 4.0f; // Critical priority during whirlwind
        }
        return 1.0f;
    }
    
    // REDUCE: Lower attack priority during whirlwind phase
    if (actionName == "melee" || actionName == "attack" || actionName.find("attack") != std::string::npos)
    {
        if (boss->HasAura(ARC_SPELL_WHIRLWIND))
        {
            return 0.1f; // Significantly reduce attack priority during whirlwind
        }
    }
    
    return 1.0f;
}

float SoccothratesMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    std::string actionName = action->getName();
    
    // EMERGENCY: Boost knock away avoidance
    if (actionName == "soccothrates knock away")
    {
        if (boss->FindCurrentSpellBySpellId(ARC_SPELL_KNOCK_AWAY))
        {
            return 3.5f; // High priority during cast
        }
        return 1.0f;
    }
    
    // EMERGENCY: Boost charge spreading
    if (actionName == "soccothrates charge")
    {
        if (boss->HasAura(SPELL_FELFIRE) || boss->FindCurrentSpellBySpellId(ARC_SPELL_CHARGE))
        {
            return 3.0f; // High priority to spread from allies
        }
        return 1.0f;
    }
    
    // EMERGENCY: Boost felfire ground avoidance
    if (actionName == "avoid felfire ground")
    {
        Player* bot = botAI->GetBot();
        if (!bot) return 1.0f;
        
        if (bot->GetHealthPct() < 85.0f) // Taking damage from fire
        {
            return 4.0f; // Critical priority when taking damage
        }
        return 2.5f; // Always elevated during encounter
    }
    
    return 1.0f;
}

float SkyrissMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    std::string actionName = action->getName();
    
    // CRITICAL: Illusion targeting - highest priority
    if (actionName == "skyriss illusion")
    {
        // Check if illusions are present
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
            {
                return 6.0f; // Critical - illusions must die first
            }
        }
        return 1.0f;
    }
    
    // EMERGENCY: Fear management
    if (actionName == "skyriss fear")
    {
        Player* bot = botAI->GetBot();
        if (bot && bot->HasAura(ARC_SPELL_FEAR))
        {
            return 4.0f; // High priority to break fear
        }
        if (boss->FindCurrentSpellBySpellId(ARC_SPELL_FEAR))
        {
            return 2.5f; // Spread to avoid chain fear
        }
        return 1.0f;
    }
    
    // EMERGENCY: Domination avoidance
    if (actionName == "skyriss domination")
    {
        if (boss->FindCurrentSpellBySpellId(ARC_SPELL_DOMINATION))
        {
            return 3.0f; // High priority to avoid mind control
        }
        return 1.0f;
    }
    
    // REDUCE: Lower regular attacks when illusions are present
    if (actionName == "melee" || actionName == "attack" || actionName.find("attack") != std::string::npos)
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
            {
                return 1.0f; // Do not suppress baseline attacks; Skyriss action already prioritizes illusions
            }
        }
    }
    
    return 1.0f;
}

float MellicharAddMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for any Mellichar spawned add present
    const uint32 mellicharAdds[] = {
        NPC_TRICKSTER, NPC_PH_HUNTER,      // Wave 1: Random
        NPC_AKKIRIS, NPC_SULFURON,         // Wave 3: Random  
        NPC_TW_DRAK, NPC_BL_DRAK           // Wave 4: Random
    };
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            for (uint32 addId : mellicharAdds)
            {
                if (unit->GetEntry() == addId)
                {
                    return 0.0f; // Block DpsAssist when any Mellichar add present
                }
            }
        }
    }
    return 1.0f;
}
