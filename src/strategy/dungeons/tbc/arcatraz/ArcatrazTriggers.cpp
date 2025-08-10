#include "Playerbots.h"
#include "ArcatrazTriggers.h"
#include "ArcatrazActions.h"

bool ZerekethVoidZoneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
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
        
        if (unit->GetName() == "Void Zone")
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 10.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool ZerekethShadowNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_SHADOW_NOVA))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 20.0f;
    }
    
    return false;
}

bool ZerekethSeedOfCorruptionTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_SEED_OF_CORRUPTION))
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

bool DalliahWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_WHIRLWIND) || boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 8.0f;
    }
    
    return false;
}

bool DalliahHealTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_HEAL);
}

bool SoccothratesKnockAwayTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_KNOCK_AWAY))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 15.0f;
    }
    
    return false;
}

bool SoccothratesChargeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_FELFIRE) || boss->FindCurrentSpellBySpellId(SPELL_CHARGE))
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
                return true;
            }
        }
    }
    
    return false;
}

bool SkyrissIllusionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
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
        
        if (unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
        {
            return true;
        }
    }
    
    return false;
}

bool SkyrissFearTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_FEAR);
}

bool SkyrissDominationTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_DOMINATION) || bot->HasAura(SPELL_DOMINATION);
}