#include "Playerbots.h"
#include "BotanicaMultipliers.h"
#include "BotanicaActions.h"
#include "SharedDefines.h"

float SarannisMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float FreywinnMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float LajMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    // Check if action is a spell cast
    std::string actionName = action->getName();
    
    // Check Laj's current immunity
    if (boss->HasAura(SPELL_DAMAGE_IMMUNE_SHADOW))
    {
        // Immune to shadow - avoid shadow spells
        if (actionName.find("shadow") != std::string::npos || 
            actionName.find("curse") != std::string::npos ||
            actionName.find("affliction") != std::string::npos)
        {
            return 0.0f; // Don't use shadow spells
        }
    }
    else if (boss->HasAura(SPELL_DAMAGE_IMMUNE_ARCANE))
    {
        // Immune to arcane - avoid arcane spells
        if (actionName.find("arcane") != std::string::npos ||
            actionName.find("polymorph") != std::string::npos)
        {
            return 0.0f; // Don't use arcane spells
        }
    }
    else if (boss->HasAura(SPELL_DAMAGE_IMMUNE_FIRE))
    {
        // Immune to fire - avoid fire spells
        if (actionName.find("fire") != std::string::npos ||
            actionName.find("pyroblast") != std::string::npos ||
            actionName.find("scorch") != std::string::npos ||
            actionName.find("immolate") != std::string::npos)
        {
            return 0.0f; // Don't use fire spells
        }
    }
    else if (boss->HasAura(SPELL_DAMAGE_IMMUNE_FROST))
    {
        // Immune to frost - avoid frost spells
        if (actionName.find("frost") != std::string::npos ||
            actionName.find("blizzard") != std::string::npos ||
            actionName.find("ice") != std::string::npos)
        {
            return 0.0f; // Don't use frost spells
        }
    }
    else if (boss->HasAura(SPELL_DAMAGE_IMMUNE_NATURE))
    {
        // Immune to nature - avoid nature spells
        if (actionName.find("lightning") != std::string::npos ||
            actionName.find("earth") != std::string::npos ||
            actionName.find("wrath") != std::string::npos ||
            actionName.find("moonfire") != std::string::npos ||
            actionName.find("shock") != std::string::npos)
        {
            return 0.0f; // Don't use nature spells
        }
    }
    
    // Check for adds to prioritize
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
            
        uint32 entry = unit->GetEntry();
        if (entry == NPC_THORN_LASHER || entry == NPC_THORN_FLAYER)
        {
            // Increase priority for add-related actions
            if (actionName == "laj adds priority")
            {
                return 1.0f;
            }
        }
    }
    
    return 1.0f;
}

float ThorngrinMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float WarpSplinterMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}