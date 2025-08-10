#include "Playerbots.h"
#include "ArcatrazActions.h"
#include "ArcatrazStrategy.h"

bool AvoidVoidZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss)
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
                return MoveAway(unit, 12.0f - distance);
            }
        }
    }
    
    return false;
}

bool AvoidShadowNovaAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_SHADOW_NOVA))
    {
        float safeDistance = 20.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool SeedOfCorruptionDispelAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_SEED_OF_CORRUPTION))
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

bool DalliahWhirlwindAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_WHIRLWIND) || boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        float safeDistance = 8.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool DalliahHealInterruptAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_HEAL))
    {
        // Interrupt the boss's heal
        bot->InterruptSpell(CURRENT_CHANNELED_SPELL);
        return true;
    }
    
    return false;
}

bool SoccothratesKnockAwayAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_KNOCK_AWAY))
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

bool SoccothratesChargeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss)
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
                return MoveAway(unit, 10.0f - distance);
            }
        }
    }
    
    return false;
}

bool SkyrissIllusionAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool SkyrissIllusionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
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
        
        if (unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
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

bool SkyrissFearAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_FEAR))
    {
        return false;
    }
    
    return false;
}

bool SkyrissDominationAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_DOMINATION))
    {
        return false;
    }
    
    return false;
}