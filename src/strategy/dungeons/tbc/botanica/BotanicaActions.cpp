#include "Playerbots.h"
#include "BotanicaActions.h"
#include "BotanicaStrategy.h"
#include "Spell.h"
#include "SharedDefines.h"
#include "Group.h"

bool SarannisResonanceDispelAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_ARCANE_RESONANCE))
    {
        // This debuff needs to be handled by dispel/cleanse abilities
        // The action triggers but actual dispel is handled by class-specific logic
        return true;
    }
    
    return false;
}

bool SarannisReinforcementsAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool SarannisReinforcementsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_BLOODWARDER_MENDER)
        {
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_BLOODWARDER_RESERVIST)
        {
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    return false;
}

bool FreywinnFrayerPriorityAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool FreywinnFrayerPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_TREE_FORM))
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_FRAYER && unit->IsAlive())
        {
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    return false;
}

bool FreywinnTranquilityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss)
    {
        return false;
    }
    
    // During Tree Form, Freywinn channels Tranquility to heal
    // Kill all Frayers first to break him out of Tree Form
    if (boss->HasAura(SPELL_TREE_FORM))
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit)
            {
                continue;
            }
            
            if (unit->GetEntry() == NPC_FRAYER && unit->IsAlive())
            {
                // Frayers still alive, can't break Tree Form yet
                // Focus on killing Frayers instead
                return false;
            }
        }
        
        // All Frayers dead but boss still in Tree Form
        // This shouldn't happen but handle it anyway
        return true;
    }
    
    return false;
}

bool LajAllergicReactionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_ALLERGIC_REACTION))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit)
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 10.0f)
            {
                return MoveAway(unit, 12.0f - distance);
            }
        }
    }
    
    return false;
}

bool LajTeleportPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_TELEPORT_SELF))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit)
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f)
            {
                return MoveAway(unit, 10.0f - distance);
            }
        }
    }
    
    return false;
}

bool LajAddsPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Priority: Thorn Flayers > Thorn Lashers > Boss
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    Unit* bestTarget = nullptr;
    float closestDistance = 40.0f;
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    // First priority: Thorn Flayers
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_THORN_FLAYER)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < closestDistance)
            {
                bestTarget = unit;
                closestDistance = distance;
            }
        }
    }
    
    // Second priority: Thorn Lashers if no Flayers found
    if (!bestTarget)
    {
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == NPC_THORN_LASHER)
            {
                float distance = bot->GetExactDist2d(unit);
                if (distance < closestDistance)
                {
                    bestTarget = unit;
                    closestDistance = distance;
                }
            }
        }
    }
    
    // Switch to add if found and not current target
    if (bestTarget && bestTarget != currentTarget)
    {
        return Attack(bestTarget);
    }
    
    return false;
}

bool ThorngrinSacrificeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss)
    {
        return false;
    }
    
    // When a player has Sacrifice, they are stunned and taking damage
    // Healers need to focus heal the sacrificed target
    if (bot->HasAura(SPELL_SACRIFICE))
    {
        // Bot is sacrificed - can't do anything while stunned
        // Just wait for the effect to end (8 seconds)
        return false;
    }
    
    // Check if any group member has Sacrifice and needs healing
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == bot)
                continue;
                
            if (member->HasAura(SPELL_SACRIFICE))
            {
                // Alert healers to prioritize this target
                // The healing logic is handled by healer AI
                return false;
            }
        }
    }
    
    return false;
}

bool ThorngrinHellfireAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss)
    {
        return false;
    }
    
    // Check if boss is channeling Hellfire
    bool isChannelingHellfire = false;
    if (Spell* channelledSpell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        if (channelledSpell->m_spellInfo->Id == SPELL_HELLFIRE)
        {
            isChannelingHellfire = true;
        }
    }
    
    // Also check regular cast and aura
    if (isChannelingHellfire || boss->HasAura(SPELL_HELLFIRE) || boss->FindCurrentSpellBySpellId(SPELL_HELLFIRE))
    {
        float safeDistance = 15.0f; // Increased safe distance
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 5.0f); // Move further away
        }
    }
    
    return false;
}

bool WarpSplinterWarStompAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_WAR_STOMP))
    {
        float safeDistance = 15.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool WarpSplinterArcaneVolleyAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss)
    {
        return false;
    }
    
    // Arcane Volley is instant cast, can't be interrupted
    // Instead, spread out to minimize damage
    if (boss->FindCurrentSpellBySpellId(SPELL_ARCANE_VOLLEY))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit)
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f)
            {
                return MoveAway(unit, 10.0f - distance);
            }
        }
    }
    
    return false;
}

bool WarpSplinterSaplingsPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Priority: Kill Saplings quickly before they heal the boss
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    Unit* bestTarget = nullptr;
    float closestDistance = 40.0f;
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_SAPLING)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < closestDistance)
            {
                bestTarget = unit;
                closestDistance = distance;
            }
        }
    }
    
    // Switch to Sapling if found and not current target
    if (bestTarget && bestTarget != currentTarget)
    {
        return Attack(bestTarget);
    }
    
    return false;
}