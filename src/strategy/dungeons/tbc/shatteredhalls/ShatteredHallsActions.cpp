#include "Playerbots.h"
#include "ShatteredHallsActions.h"
#include "ShatteredHallsStrategy.h"

bool AvoidShadowFissureAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }

    float safeDistance = 8.0f;
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->HasAura(SPELL_LESSER_SHADOW_FISSURE))
        {
            float currentDist = bot->GetExactDist2d(unit);
            if (currentDist < safeDistance)
            {
                // EMERGENCY: Move away from Lesser Shadow Fissure
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = safeDistance - currentDist + 2.0f;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    if (boss->HasAura(SPELL_SHADOW_FISSURE) || boss->FindCurrentSpellBySpellId(SPELL_SHADOW_FISSURE))
    {
        float currentDist = bot->GetExactDist2d(boss);
        if (currentDist < safeDistance)
        {
            // EMERGENCY: Move away from Shadow Fissure on boss
            float angle = bot->GetAngle(boss) + M_PI;
            float moveDistance = safeDistance - currentDist + 2.0f;
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool AvoidDarkSpinAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_DARK_SPIN) && !boss->FindCurrentSpellBySpellId(SPELL_DARK_SPIN))
    {
        return false;
    }
    
    float safeDistance = 10.0f;
    float currentDist = bot->GetExactDist2d(boss);
    
    if (currentDist < safeDistance)
    {
        // EMERGENCY: Move away from Dark Spin
        float angle = bot->GetAngle(boss) + M_PI;
        float moveDistance = safeDistance - currentDist + 2.0f;
        float x = bot->GetPositionX() + cos(angle) * moveDistance;
        float y = bot->GetPositionY() + sin(angle) * moveDistance;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool MoveFromBlastWaveAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss)
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_BURNING_MAUL))
    {
        return false;
    }
    
    float safeDistance = 12.0f;
    float currentDist = bot->GetExactDist2d(boss);
    
    if (currentDist < safeDistance)
    {
        // EMERGENCY: Move away from Blast Wave
        float angle = bot->GetAngle(boss) + M_PI;
        float moveDistance = safeDistance - currentDist + 2.0f;
        float x = bot->GetPositionX() + cos(angle) * moveDistance;
        float y = bot->GetPositionY() + sin(angle) * moveDistance;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool AvoidBurningMaulAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_FEAR))
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_BURNING_MAUL))
    {
        float safeDistance = 8.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (botAI->IsTank(bot))
        {
            if (currentDist > 5.0f)
            {
                return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
                            false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
        else if (currentDist < safeDistance)
        {
            // EMERGENCY: Move away from Burning Maul
            float angle = bot->GetAngle(boss) + M_PI;
            float moveDistance = safeDistance - currentDist + 2.0f;
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool AvoidBladeDanceAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_BLADE_DANCE_DMG) || boss->FindCurrentSpellBySpellId(SPELL_BLADE_DANCE_DMG))
    {
        float safeDistance = 10.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            // EMERGENCY: Move away from Blade Dance
            float angle = bot->GetAngle(boss) + M_PI;
            float moveDistance = safeDistance - currentDist + 2.0f;
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool KillShatteredAssassinsAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool KillShatteredAssassinsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector attackers = AI_VALUE(GuidVector, "attackers");
    for (auto& attacker : attackers)
    {
        Unit* unit = botAI->GetUnit(attacker);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_SHATTERED_ASSASSIN)
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

bool NetheKursePeonPriorityAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool NetheKursePeonPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
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
        
        if (unit->GetEntry() == NPC_PEON && unit->IsAlive() && unit->IsInCombat())
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
bool AvoidFlameArrowFireAction::Execute(Event event)
{
    if (!IsFireNearby())
    {
        return false;
    }
    
    Position safePos = GetSafePosition();
    return MoveTo(bot->GetMapId(), safePos.GetPositionX(), safePos.GetPositionY(), 
                  safePos.GetPositionZ(), false, false, false, true, 
                  MovementPriority::MOVEMENT_FORCED);
}

bool AvoidFlameArrowFireAction::isUseful()
{
    return IsFireNearby();
}

bool AvoidFlameArrowFireAction::IsFireNearby()
{
    // Check for fire effects or triggers in the area
    std::list<GameObject*> gameObjects;
    bot->GetGameObjectListWithEntryInGrid(gameObjects, 182592, 10.0f); // Fire visual objects
    
    if (!gameObjects.empty())
    {
        return true;
    }
    
    // Check for units with fire aura or casting fire
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Check if this is a fire effect trigger or has fire aura
        if (unit->HasAura(SPELL_FLAME_ARROW_FIRE))
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
        
        // Check for invisible triggers that represent fire patches
        if (unit->GetEntry() == 17662 || unit->GetEntry() == 18370) // Common fire trigger IDs
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }
    
    // Check if archers are actively shooting at our position
    GuidVector hostiles = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : hostiles)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit->GetEntry() != NPC_SH_ARCHER)
            continue;
            
        if (unit->FindCurrentSpellBySpellId(SPELL_SHOOT_FLAME_ARROW))
        {
            if (unit->GetVictim() == bot)
                return true; // Arrow incoming to our position
        }
    }
    
    return false;
}

Position AvoidFlameArrowFireAction::GetSafePosition()
{
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafeDist = 0;
    
    // Try to find a safe spot by checking multiple directions
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 4)
    {
        float dist = 10.0f;
        float newX = bot->GetPositionX() + cos(angle) * dist;
        float newY = bot->GetPositionY() + sin(angle) * dist;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        // Check if this position is safe from fire
        bool isSafe = true;
        float minFireDist = 999.0f;
        
        // Check distance from fire sources
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit)
                continue;
                
            if (unit->HasAura(SPELL_FLAME_ARROW_FIRE) || 
                unit->GetEntry() == 17662 || unit->GetEntry() == 18370)
            {
                float fireDist = unit->GetDistance2d(newX, newY);
                if (fireDist < 8.0f)
                {
                    isSafe = false;
                    break;
                }
                if (fireDist < minFireDist)
                    minFireDist = fireDist;
            }
        }
        
        // Also check for game objects
        std::list<GameObject*> gameObjects;
        bot->GetGameObjectListWithEntryInGrid(gameObjects, 182592, 20.0f);
        for (auto* obj : gameObjects)
        {
            float fireDist = obj->GetDistance2d(newX, newY);
            if (fireDist < 8.0f)
            {
                isSafe = false;
                break;
            }
            if (fireDist < minFireDist)
                minFireDist = fireDist;
        }
        
        if (isSafe && minFireDist > maxSafeDist)
        {
            maxSafeDist = minFireDist;
            bestX = newX;
            bestY = newY;
            bestZ = newZ;
        }
    }
    
    // If we are in the gauntlet, prefer moving forward rather than backward
    if (bot->GetPositionX() > -50.0f && bot->GetPositionX() < 50.0f) // Gauntlet corridor range
    {
        // Bias toward moving forward (positive X direction in Shattered Halls)
        bestX += 3.0f;
    }
    
    return Position(bestX, bestY, bestZ, bot->GetOrientation());
}
