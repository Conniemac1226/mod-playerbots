#include "AuchenaiCryptsActions.h"
#include "AuchenaiCryptsTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "Value.h"

// Per-bot state map for preventing repeated movements during Focus Fire phase
std::map<ObjectGuid, bool> g_shirrak_inSafePosition;

bool ShirrakFocusFireAvoidAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();

    // Already in safe position for this phase
    if (g_shirrak_inSafePosition[botGuid])
        return true;

    // PRIORITY 1: Check for Focus Fire creature near us (spawns 3 seconds before damage)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 60.0f);

    Unit* nearestFocusFire = nullptr;
    float nearestDistance = 60.0f;
    
    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            float dist = bot->GetDistance(unit);
            if (dist < nearestDistance)
            {
                nearestDistance = dist;
                nearestFocusFire = unit;
            }
        }
    }

    // If Focus Fire exists and is near us, MOVE IMMEDIATELY
    if (nearestFocusFire && nearestDistance < 15.0f)  // Within danger zone
    {
        float focusX = nearestFocusFire->GetPositionX();
        float focusY = nearestFocusFire->GetPositionY();
        float botX = bot->GetPositionX();
        float botY = bot->GetPositionY();
        float botZ = bot->GetPositionZ();
        
        // Calculate escape direction
        float dx = botX - focusX;
        float dy = botY - focusY;
        float distance = sqrt(dx * dx + dy * dy);
        
        // If we're at the exact spawn point, move in any direction
        if (distance < 1.0f) {
            // Move away from boss as default
            Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
            if (boss) {
                dx = botX - boss->GetPositionX();
                dy = botY - boss->GetPositionY();
                distance = sqrt(dx * dx + dy * dy);
            } else {
                // Random direction if no boss
                float angle = frand(0, 2 * M_PI);
                dx = cos(angle);
                dy = sin(angle);
                distance = 1.0f;
            }
        }
        
        // Move 20 yards away from Focus Fire
        float moveDistance = 20.0f;
        float safeX = focusX + (dx / distance) * moveDistance;
        float safeY = focusY + (dy / distance) * moveDistance;
        
        // Use MOVEMENT_FORCED for immediate response
        bool result = MoveTo(bot->GetMapId(), safeX, safeY, botZ, 
                            false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        
        if (!result) {
            // Try perpendicular direction if direct path failed
            float perpX = focusX - (dy / distance) * moveDistance;
            float perpY = focusY + (dx / distance) * moveDistance;
            
            result = MoveTo(bot->GetMapId(), perpX, perpY, botZ, 
                           false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        
        if (result) {
            g_shirrak_inSafePosition[botGuid] = true;
        }
        return result;
    }
    
    return false;
}

bool ShirrakFocusFireAvoidAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Already safe?
    ObjectGuid botGuid = bot->GetGUID();
    if (g_shirrak_inSafePosition[botGuid])
        return false;

    // Check if Focus Fire exists near us
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            // Focus Fire exists and we're within danger zone
            float distance = bot->GetDistance(unit);
            return distance < 15.0f;
        }
    }
    
    return false;
}

bool ShirrakReturnPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    ObjectGuid botGuid = bot->GetGUID();
    
    // Reset safe position state
    g_shirrak_inSafePosition[botGuid] = false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Return to optimal range based on class and Inhibit Magic stacks
    float optimalRange;
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || 
        bot->getClass() == CLASS_PRIEST)
    {
        // Caster optimal range (2 stacks)
        optimalRange = 27.5f;
    }
    else if (bot->getClass() == CLASS_HUNTER)
    {
        // Hunter optimal range (1 stack)
        optimalRange = 37.5f;
    }
    else
    {
        // Melee optimal range (3 stacks)
        optimalRange = 17.5f;
    }
    
    // Position around boss at optimal range
    float angle = frand(0, 2 * M_PI);
    float newX = boss->GetPositionX() + cos(angle) * optimalRange;
    float newY = boss->GetPositionY() + sin(angle) * optimalRange;
    float newZ = boss->GetPositionZ();
    
    
    return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool ShirrakReturnPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    
    // Only useful if we were previously in safe position
    if (!g_shirrak_inSafePosition[botGuid])
        return false;

    // Only useful when no Focus Fire exists
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
            return false;
    }
    
    // Safe to return to combat position
    return true;
}

bool MaladaarSoulScreamAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "exarch maladaar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // RESEARCHED: Soul Scream - boss_exarch_maladaar.cpp:96-99
    // AoE fear every 15-25 seconds
    // Spread to avoid chain fears
    if (boss->FindCurrentSpellBySpellId(SPELL_SOUL_SCREAM))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
                continue;
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive())
                continue;
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f) // Spread to avoid chain fear
            {
                // Move away from allies
                float angle = bot->GetAngle(unit) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 10.0f;
                float y = bot->GetPositionY() + sin(angle) * 10.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}

bool MaladaarStolenSoulAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "exarch maladaar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // RESEARCHED: Stolen Soul - boss_exarch_maladaar.cpp:104-119
    // Creates a copy of a player that attacks them
    // Must kill the copy quickly
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
        
        if (unit->GetEntry() == NPC_STOLEN_SOUL)
        {
            // Priority target - kill stolen souls immediately
            return Attack(unit);
        }
    }
    
    return false;
}

bool MaladaarStolenSoulAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return !botAI->IsHeal(bot);
}

bool MaladaarAvatarAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "exarch maladaar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // RESEARCHED: Avatar summoned at 25% - boss_exarch_maladaar.cpp:73-78
    // Avatar of the Martyred is powerful add that must be tanked
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
        
        if (unit->GetEntry() == NPC_AVATAR)
        {
            // Tank should pick up Avatar immediately
            if (botAI->IsTank(bot))
            {
                return Attack(unit);
            }
            // DPS can focus Avatar after tank has aggro
            else if (!botAI->IsHeal(bot))
            {
                if (unit->GetVictim() && botAI->IsTank(unit->GetVictim()->ToPlayer()))
                {
                    return Attack(unit);
                }
            }
            return false;
        }
    }
    
    return false;
}

bool MaladaarAvatarAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return !botAI->IsHeal(bot);
}

bool MaladaarRibbonOfSoulsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // RESEARCHED: Ribbon of Souls - boss_exarch_maladaar.cpp:101-103
    // Immobilizes and damages random target
    // Dispel if possible
    if (bot->HasAura(SPELL_RIBBON_OF_SOULS))
    {
        // Try to dispel or use escape abilities
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, bot, false))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }
    
    return false;
}