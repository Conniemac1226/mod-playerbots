#include "Playerbots.h"
#include "BotanicaTriggers.h"
#include "BotanicaActions.h"
#include "Spell.h"
#include "SharedDefines.h"

bool SarannisResonanceTrigger::IsActive()
{
    return bot->HasAura(SPELL_ARCANE_RESONANCE);
}

bool SarannisReinforcementsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if ((unit->GetEntry() == NPC_BLOODWARDER_MENDER || unit->GetEntry() == NPC_BLOODWARDER_RESERVIST) && unit->IsAlive())
        {
            return true;
        }
    }
    
    return false;
}

bool FreywinnFrayersTrigger::IsActive()
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
            return true;
        }
    }
    
    return false;
}

bool FreywinnTranquilityTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->HasAura(SPELL_TREE_FORM) && boss->HasAura(SPELL_TRANQUILITY);
}

bool LajAllergicReactionTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_ALLERGIC_REACTION))
    {
        return false;
    }
    
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
            return true;
        }
    }
    
    return false;
}

bool LajTeleportTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_TELEPORT_SELF);
}

bool LajAddsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Check for Laj's adds
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
            
        uint32 entry = unit->GetEntry();
        if (entry == NPC_THORN_LASHER || entry == NPC_THORN_FLAYER)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 40.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool ThorngrinSacrificeTrigger::IsActive()
{
    return bot->HasAura(SPELL_SACRIFICE);
}

bool ThorngrinHellfireTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Check if boss is channeling Hellfire
    if (Spell* channelledSpell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        if (channelledSpell->m_spellInfo->Id == SPELL_HELLFIRE)
        {
            float distance = bot->GetExactDist2d(boss);
            return distance < 15.0f; // Increased range for safety
        }
    }
    
    // Also check regular cast and aura (in case spell mechanics vary)
    if (boss->HasAura(SPELL_HELLFIRE) || boss->FindCurrentSpellBySpellId(SPELL_HELLFIRE))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 15.0f;
    }
    
    return false;
}

bool WarpSplinterWarStompTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_WAR_STOMP))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 15.0f;
    }
    
    return false;
}

bool WarpSplinterArcaneVolleyTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_ARCANE_VOLLEY);
}

bool WarpSplinterSaplingsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Check for Saplings
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_SAPLING)
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 40.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}