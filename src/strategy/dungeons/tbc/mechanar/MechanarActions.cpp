#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Per-bot state management
std::map<ObjectGuid, uint32> g_capacitus_lastPolarityTime;
std::map<ObjectGuid, bool> g_capacitus_hasPositive;
std::map<ObjectGuid, bool> g_capacitus_hasNegative;

// Mechano Lord Capacitus

bool CapacitusReflectiveShieldAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Reflective shields - alternates between magic and damage shields
    if (boss->HasAura(SPELL_REFLECTIVE_MAGIC_SHIELD))
    {
        // Stop casting spells during magic shield
        if (bot->IsNonMeleeSpellCast(false))
        {
            botAI->InterruptSpell();
            return true;
        }
        // Melee can continue attacking
    }
    else if (boss->HasAura(SPELL_REFLECTIVE_DAMAGE_SHIELD))
    {
        // Melee should stop attacking during damage shield
        if (botAI->IsMelee(bot))
        {
            bot->AttackStop();
            return true;
        }
        // Casters can continue casting
    }

    return false;
}

bool CapacitusReflectiveShieldAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->GetMap()->IsHeroic())
        return false; // Only on Normal

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    return boss && (boss->HasAura(SPELL_REFLECTIVE_MAGIC_SHIELD) || 
                   boss->HasAura(SPELL_REFLECTIVE_DAMAGE_SHIELD));
}

bool CapacitusPolarityShiftAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Polarity Shift - players with same charge stack, opposite charges spread
    
    bool hasPositive = bot->HasAura(SPELL_POSITIVE_POLARITY);
    bool hasNegative = bot->HasAura(SPELL_NEGATIVE_POLARITY);
    
    if (!hasPositive && !hasNegative)
    {
        g_capacitus_hasPositive[botGuid] = false;
        g_capacitus_hasNegative[botGuid] = false;
        return false;
    }

    // Update polarity state
    g_capacitus_hasPositive[botGuid] = hasPositive;
    g_capacitus_hasNegative[botGuid] = hasNegative;
    g_capacitus_lastPolarityTime[botGuid] = currentTime;

    // Find other players with same/opposite polarity
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    Position safePos;
    bool foundSamePolarity = false;
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        bool allyPositive = ally->HasAura(SPELL_POSITIVE_POLARITY);
        bool allyNegative = ally->HasAura(SPELL_NEGATIVE_POLARITY);
        
        if ((hasPositive && allyPositive) || (hasNegative && allyNegative))
        {
            // Same polarity - move closer to stack
            float distance = bot->GetDistance(ally);
            if (distance > 5.0f)
            {
                safePos = ally->GetPosition();
                foundSamePolarity = true;
                break;
            }
        }
        else if ((hasPositive && allyNegative) || (hasNegative && allyPositive))
        {
            // Opposite polarity - stay away
            float distance = bot->GetDistance(ally);
            if (distance < 10.0f)
            {
                // Move away from opposite polarity
                float angle = bot->GetAngle(ally) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 15.0f;
                float y = bot->GetPositionY() + sin(angle) * 15.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    if (foundSamePolarity)
    {
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                     safePos.m_positionZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool CapacitusPolarityShiftAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false; // Only on Heroic

    return bot->HasAura(SPELL_POSITIVE_POLARITY) || bot->HasAura(SPELL_NEGATIVE_POLARITY);
}

bool CapacitusNetherChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Nether Charges - priority targets for ranged DPS
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestCharge = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_CHARGE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                nearestCharge = unit;
                closestDistance = distance;
            }
        }
    }

    if (nearestCharge)
    {
        // Priority target for ranged DPS
        if (!botAI->IsMelee(bot))
        {
            return Attack(nearestCharge);
        }
    }

    return false;
}

bool CapacitusNetherChargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* chargeActiveValue = botAI->GetAiObjectContext()->GetValue<bool>("nether charge active");
    if (!chargeActiveValue)
        return false;
    
    return chargeActiveValue->Get();
}

bool CapacitusPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Spread positioning for Nether Charges
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        float distance = bot->GetDistance(ally);
        if (distance < 8.0f && !botAI->IsTank(bot))
        {
            // Spread from allies for charges
            float angle = bot->GetAngle(ally) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 10.0f;
            float y = bot->GetPositionY() + sin(angle) * 10.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool CapacitusPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* engagedValue = botAI->GetAiObjectContext()->GetValue<bool>("capacitus engaged");
    if (!engagedValue)
        return false;
    
    return engagedValue->Get();
}

// ========== NETHERMANCER SEPETHREA ACTIONS ==========

// Per-bot state maps for persistent kiting
static std::map<ObjectGuid, uint32> g_flame_lastMoveTime;
static std::map<ObjectGuid, bool> g_flame_inSafePosition;

// Room boundaries for safe movement (expanded for safety)
const Position MECHANAR_SEPETHREA_CENTER = {290.52f, 11.492f, 25.39f, 0.0f};
const float MECHANAR_ROOM_MIN_X = 272.0f;
const float MECHANAR_ROOM_MAX_X = 308.0f;
const float MECHANAR_ROOM_MIN_Y = -8.0f;
const float MECHANAR_ROOM_MAX_Y = 31.0f;
const float MECHANAR_ROOM_MIN_Z = 23.0f;
const float MECHANAR_ROOM_MAX_Z = 28.0f;

// Boundary validation to prevent out-of-bounds movement with buffer
bool IsPositionSafe(const Position& pos)
{
    // Add 2 yard buffer from walls to prevent clipping
    const float buffer = 2.0f;
    return pos.m_positionX >= MECHANAR_ROOM_MIN_X + buffer && pos.m_positionX <= MECHANAR_ROOM_MAX_X - buffer &&
           pos.m_positionY >= MECHANAR_ROOM_MIN_Y + buffer && pos.m_positionY <= MECHANAR_ROOM_MAX_Y - buffer &&
           pos.m_positionZ >= MECHANAR_ROOM_MIN_Z && pos.m_positionZ <= MECHANAR_ROOM_MAX_Z;
}

// Validate movement path doesn't go through walls
bool IsPathClear(const Position& from, const Position& to)
{
    const int steps = 10; // Check 10 points along the path
    for (int i = 1; i <= steps; ++i)
    {
        float t = float(i) / float(steps);
        Position checkPos;
        checkPos.m_positionX = from.m_positionX + t * (to.m_positionX - from.m_positionX);
        checkPos.m_positionY = from.m_positionY + t * (to.m_positionY - from.m_positionY);
        checkPos.m_positionZ = from.m_positionZ + t * (to.m_positionZ - from.m_positionZ);
        
        if (!IsPositionSafe(checkPos))
            return false;
    }
    return true;
}

// Force position to stay within safe room bounds
Position ConstrainToRoom(const Position& pos)
{
    const float buffer = 3.0f;
    Position safePos = pos;
    safePos.m_positionX = std::max(MECHANAR_ROOM_MIN_X + buffer, 
                                  std::min(MECHANAR_ROOM_MAX_X - buffer, pos.m_positionX));
    safePos.m_positionY = std::max(MECHANAR_ROOM_MIN_Y + buffer, 
                                  std::min(MECHANAR_ROOM_MAX_Y - buffer, pos.m_positionY));
    safePos.m_positionZ = std::max(MECHANAR_ROOM_MIN_Z, 
                                  std::min(MECHANAR_ROOM_MAX_Z, pos.m_positionZ));
    return safePos;
}

bool SepethreaRagingFlamesAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Find Raging Flames targeting this bot
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* targetingFlame = nullptr;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (flame->GetVictim() == bot)
        {
            targetingFlame = flame;
            break;
        }
    }
    
    if (!targetingFlame)
    {
        g_flame_inSafePosition[botGuid] = false;
        return false;
    }

    float flameDistance = bot->GetDistance(targetingFlame);
    
    // Stop spellcasting when kiting flames
    if (bot->IsNonMeleeSpellCast(false))
        botAI->InterruptSpell();
    
    // Emergency escape from fire trail
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA))
    {
        botAI->Reset();
        return FleePosition(bot->GetPosition(), 12.0f, 100U);
    }
    
    // SIMPLE RECTANGULAR KITING - adapted from ICC ooze pattern
    if (flameDistance < 20.0f)
    {
        botAI->Reset();
        
        const Position centerPos = MECHANAR_SEPETHREA_CENTER;
        const float roomWidth = MECHANAR_ROOM_MAX_X - MECHANAR_ROOM_MIN_X;
        const float roomHeight = MECHANAR_ROOM_MAX_Y - MECHANAR_ROOM_MIN_Y;
        
        // Safe zones within room boundaries (4 yard buffer from walls)
        const float safeMinX = MECHANAR_ROOM_MIN_X + 4.0f;
        const float safeMaxX = MECHANAR_ROOM_MAX_X - 4.0f;
        const float safeMinY = MECHANAR_ROOM_MIN_Y + 4.0f;
        const float safeMaxY = MECHANAR_ROOM_MAX_Y - 4.0f;
        
        Position botPos = bot->GetPosition();
        
        // Calculate current angle from center to bot
        float currentAngle = atan2(botPos.m_positionY - centerPos.GetPositionY(), 
                                  botPos.m_positionX - centerPos.GetPositionX());
        
        // Move counterclockwise around room perimeter at safe distance
        float moveAngle = currentAngle + 0.8f; // Larger step for faster movement
        
        // Calculate ideal position on rectangular orbit
        float targetX, targetY;
        
        // For narrow room, use elliptical movement pattern
        float radiusX = (roomWidth * 0.35f); // 35% of width
        float radiusY = (roomHeight * 0.4f);  // 40% of height
        
        targetX = centerPos.GetPositionX() + cos(moveAngle) * radiusX;
        targetY = centerPos.GetPositionY() + sin(moveAngle) * radiusY;
        
        // Clamp to safe boundaries
        targetX = std::max(safeMinX, std::min(safeMaxX, targetX));
        targetY = std::max(safeMinY, std::min(safeMaxY, targetY));
        
        Position targetPos(targetX, targetY, bot->GetPositionZ(), 0.0f);
        
        // WALL COLLISION VALIDATION - ensure path is clear
        if (IsPositionSafe(targetPos) && IsPathClear(botPos, targetPos))
        {
            // Ensure minimum distance from flame
            float distanceToFlame = sqrt(pow(targetX - targetingFlame->GetPositionX(), 2) + 
                                       pow(targetY - targetingFlame->GetPositionY(), 2));
            
            if (distanceToFlame >= 15.0f)
            {
                g_flame_lastMoveTime[botGuid] = currentTime;
                return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(),
                             false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
        
        // Fallback: simple flee if orbit position too close to flame
        return FleePosition(targetingFlame->GetPosition(), 18.0f, 200U);
    }
    else if (flameDistance > 25.0f)
    {
        // Safe distance reached
        g_flame_inSafePosition[botGuid] = true;
        g_flame_lastMoveTime[botGuid] = currentTime;
    }

    return false;
}

bool SepethreaRagingFlamesAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Use direct GUID search for flame detection
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (flame->GetVictim() == bot)
            return true;
    }
    
    return false;
}

bool SepethreaDragonsBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Dragon's Breath - frontal cone attack
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_DRAGONS_BREATH))
    {
        // Check if in frontal cone
        if (boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot))
        {
            // Calculate side escape direction
            float bossOrientation = boss->GetOrientation();
            float botAngleToBoss = boss->GetAngle(bot);
            
            // Choose left or right side
            float leftAngle = bossOrientation - (M_PI / 2);
            float rightAngle = bossOrientation + (M_PI / 2);
            
            // Choose closer side
            float angleDiffLeft = fabs(botAngleToBoss - leftAngle);
            float angleDiffRight = fabs(botAngleToBoss - rightAngle);
            
            float escapeAngle = (angleDiffLeft < angleDiffRight) ? leftAngle : rightAngle;
            
            // Move to safe position at boss side
            float escapeDistance = 15.0f;
            Position safePos;
            safePos.m_positionX = boss->GetPositionX() + cos(escapeAngle) * escapeDistance;
            safePos.m_positionY = boss->GetPositionY() + sin(escapeAngle) * escapeDistance;
            safePos.m_positionZ = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                         safePos.m_positionZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool SepethreaDragonsBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* breathValue = botAI->GetAiObjectContext()->GetValue<bool>("dragons breath danger");
    if (!breathValue)
        return false;
    
    return breathValue->Get();
}


bool SepethreaArcaneBlastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Blast - threat reduction, tanks need to taunt back
    if (botAI->IsTank(bot))
    {
        if (boss->GetVictim() && boss->GetVictim() != bot)
        {
            // Check for Arcane Blast cast
            if (boss->HasUnitState(UNIT_STATE_CASTING) && 
                boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_BLAST))
            {
                // Taunt after cast
                return botAI->CastSpell("taunt", boss);
            }
        }
    }

    return false;
}

bool SepethreaArcaneBlastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || !botAI->IsTank(bot))
        return false;

    Value<bool>* engagedValue = botAI->GetAiObjectContext()->GetValue<bool>("sepethrea engaged");
    if (!engagedValue)
        return false;
    
    return engagedValue->Get();
}

bool SepethreaTargetElementalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (bot->GetSelectedUnit() != boss)
    {
        bot->SetSelection(boss->GetGUID());
    }

    return true;
}

bool SepethreaTargetElementalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false;

    // Check for Raging Flames targeting bot
    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss)
        return false;

    // Check for Raging Flames
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool hasFlameTargetingBot = false;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive())
            continue;
            
        if (flame->GetEntry() == NPC_RAGING_FLAMES && flame->GetVictim() == bot)
        {
            hasFlameTargetingBot = true;
            break;
        }
    }
    
    if (!hasFlameTargetingBot)
        return false;

    // Only trigger if not already targeting boss
    return bot->GetSelectedUnit() != boss;
}

bool SepethreaInfernoAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* infernoFlame = nullptr;
    float closestDistance = 20.0f;

    // Find closest Raging Flame casting Inferno
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        if (unit->FindCurrentSpellBySpellId(SPELL_INFERNO))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                infernoFlame = unit;
                closestDistance = distance;
            }
        }
    }

    if (!infernoFlame)
        return false;

    // Flee from Inferno
    return FleePosition(infernoFlame->GetPosition(), 18.0f, 300U);
}

bool SepethreaInfernoAvoidanceAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        // Check for Inferno casting
        if (unit->FindCurrentSpellBySpellId(SPELL_INFERNO) && bot->GetDistance(unit) < 18.0f)
            return true;
    }
    
    return false;
}

// Fire trail avoidance for all bots
bool SepethreaFireTrailAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // IMMEDIATE emergency response to standing in fire trail
    if (bot->IsNonMeleeSpellCast(false))
        botAI->InterruptSpell();
    
    botAI->Reset();
    
    // Find nearest safe spot away from any fire trail areas
    const Position centerPos = MECHANAR_SEPETHREA_CENTER;
    Position botPos = bot->GetPosition();
    
    // Calculate direction to room center
    float dirX = centerPos.GetPositionX() - botPos.m_positionX;
    float dirY = centerPos.GetPositionY() - botPos.m_positionY;
    float length = sqrt(dirX * dirX + dirY * dirY);
    
    if (length > 0.1f)
    {
        // Normalize and move toward center
        dirX /= length;
        dirY /= length;
        
        float targetX = botPos.m_positionX + dirX * 8.0f;
        float targetY = botPos.m_positionY + dirY * 8.0f;
        
        // Ensure within room bounds
        targetX = std::max(MECHANAR_ROOM_MIN_X + 3.0f, std::min(MECHANAR_ROOM_MAX_X - 3.0f, targetX));
        targetY = std::max(MECHANAR_ROOM_MIN_Y + 3.0f, std::min(MECHANAR_ROOM_MAX_Y - 3.0f, targetY));
        
        Position targetPos(targetX, targetY, bot->GetPositionZ(), 0.0f);
        
        // WALL COLLISION CHECK - validate path is clear
        if (IsPositionSafe(targetPos) && IsPathClear(botPos, targetPos))
        {
            return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // Fallback: quick flee from current position
    return FleePosition(bot->GetPosition(), 10.0f, 100U);
}

bool SepethreaFireTrailAvoidanceAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if standing in fire trail aura
    return bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA);
}

// ========== PATHALEON THE CALCULATOR ACTIONS ==========

bool PathaleonDominationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Domination is mind control - affected players need to be CC'd
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        if (ally->HasAura(MECH_SPELL_DOMINATION))
        {
            // CC the mind controlled ally
            if (bot->getClass() == CLASS_MAGE)
            {
                return botAI->CastSpell("polymorph", ally);
            }
            else if (bot->getClass() == CLASS_PRIEST)
            {
                return botAI->CastSpell("shackle undead", ally);
            }
            else if (bot->getClass() == CLASS_WARLOCK)
            {
                return botAI->CastSpell("fear", ally);
            }
        }
    }

    // If we are dominated, can't do anything
    if (bot->HasAura(MECH_SPELL_DOMINATION))
    {
        return true;
    }

    return false;
}

bool PathaleonDominationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* dominationValue = botAI->GetAiObjectContext()->GetValue<bool>("domination active");
    if (!dominationValue)
        return false;
    
    return dominationValue->Get();
}

bool PathaleonNetherWraithAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Summons 3-4 Nether Wraiths that need to be killed quickly
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestWraith = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_WRAITH)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                nearestWraith = unit;
                closestDistance = distance;
            }
        }
    }

    if (nearestWraith)
    {
        return Attack(nearestWraith);
    }

    return false;
}

bool PathaleonNetherWraithAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* wraithValue = botAI->GetAiObjectContext()->GetValue<bool>("nether wraith active");
    if (!wraithValue)
        return false;
    
    return wraithValue->Get();
}

bool PathaleonArcaneTorrentAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Torrent is an AoE around the boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_TORRENT))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f && !botAI->IsMelee(bot))
        {
            // Move out of Arcane Torrent range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool PathaleonArcaneTorrentAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* torrentValue = botAI->GetAiObjectContext()->GetValue<bool>("arcane torrent danger");
    if (!torrentValue)
        return false;
    
    return torrentValue->Get();
}

bool PathaleonEnrageAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // At 20% health: Frenzy + Suicide (explodes on death)
    if (boss->HasAura(MECH_SPELL_FRENZY))
    {
        // Boss will explode on death - prepare to run
        if (boss->GetHealthPct() < 5.0f)
        {
            // Get ready to flee when he dies (suicide explosion)
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
        
        // Healers focus on tank during enrage
        if (botAI->IsHeal(bot))
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->IsPlayer() && botAI->IsTank(ally->ToPlayer()))
                {
                    if (ally->GetHealthPct() < 70.0f)
                    {
                        return botAI->CastSpell("heal", ally);
                    }
                }
            }
        }
        
        // DPS burn hard
        if (!botAI->IsTank(bot) && !botAI->IsHeal(bot))
        {
            return botAI->CastSpell("boost", bot);
        }
    }

    return false;
}

bool PathaleonEnrageAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    return boss && boss->HasAura(MECH_SPELL_FRENZY);
}

bool PathaleonArcaneExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Explosion on Heroic - AoE around boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_EXPLOSION))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 10.0f && !botAI->IsMelee(bot))
        {
            // Move out of Arcane Explosion range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool PathaleonArcaneExplosionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || !bot->GetMap()->IsHeroic())
        return false;

    Value<bool>* explosionValue = botAI->GetAiObjectContext()->GetValue<bool>("arcane explosion danger");
    if (!explosionValue)
        return false;
    
    return explosionValue->Get();
}

bool PathaleonManaTapAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Mana Tap drains mana from a random player
    if (bot->HasAura(MECH_SPELL_MANA_TAP))
    {
        // Affected player should use mana efficiently
        // Healers prioritize emergency heals only
        if (botAI->IsHeal(bot))
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->GetHealthPct() < 30.0f)
                {
                    return botAI->CastSpell("heal", ally);
                }
            }
        }
    }

    return false;
}

bool PathaleonManaTapAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return bot && bot->getPowerType() == POWER_MANA && bot->HasAura(MECH_SPELL_MANA_TAP);
}