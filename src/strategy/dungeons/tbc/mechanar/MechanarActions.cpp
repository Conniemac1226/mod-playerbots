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

// Boundary validation to prevent out-of-bounds movement
bool IsPositionSafe(const Position& pos)
{
    return pos.m_positionX >= MECHANAR_ROOM_MIN_X && pos.m_positionX <= MECHANAR_ROOM_MAX_X &&
           pos.m_positionY >= MECHANAR_ROOM_MIN_Y && pos.m_positionY <= MECHANAR_ROOM_MAX_Y &&
           pos.m_positionZ >= MECHANAR_ROOM_MIN_Z && pos.m_positionZ <= MECHANAR_ROOM_MAX_Z;
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
        if (!botAI)
            continue;
            
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || !flame->IsInWorld())
            continue;
            
        if (flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        Unit* victim = flame->GetVictim();
        if (victim && victim == bot)
        {
            targetingFlame = flame;
            break;
        }
    }
    
    if (!targetingFlame)
    {
        // Reset state when no flame is targeting
        g_flame_inSafePosition[botGuid] = false;
        return false;
    }

    // Validate flame is still valid before using
    if (!targetingFlame->IsAlive() || !targetingFlame->IsInWorld())
    {
        g_flame_inSafePosition[botGuid] = false;
        return false;
    }

    float flameDistance = bot->GetDistance(targetingFlame);
    
    // Stop spellcasting when kiting flames
    if (bot->IsNonMeleeSpellCast(false))
    {
        if (botAI)
        {
            botAI->InterruptSpell();
        }
    }
    
    // Emergency escape from fire trail
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA))
    {
        if (botAI)
        {
            botAI->Reset();
        }
        return FleePosition(bot->GetPosition(), 8.0f, 200U);
    }
    
    // Kite Raging Flames at safe distance
    if (flameDistance < 18.0f)
    {
        if (botAI)
        {
            botAI->Reset();
        }
        
        // Wall collision detection
        Position botPos = bot->GetPosition();
        bool nearWall = !IsPositionSafe(Position(botPos.m_positionX + 3.0f, botPos.m_positionY, botPos.m_positionZ, 0.0f)) ||
                       !IsPositionSafe(Position(botPos.m_positionX - 3.0f, botPos.m_positionY, botPos.m_positionZ, 0.0f)) ||
                       !IsPositionSafe(Position(botPos.m_positionX, botPos.m_positionY + 3.0f, botPos.m_positionZ, 0.0f)) ||
                       !IsPositionSafe(Position(botPos.m_positionX, botPos.m_positionY - 3.0f, botPos.m_positionZ, 0.0f));
        
        // Check proximity to room boundaries
        bool veryNearWall = (botPos.m_positionX <= MECHANAR_ROOM_MIN_X + 2.0f) ||
                           (botPos.m_positionX >= MECHANAR_ROOM_MAX_X - 2.0f) ||
                           (botPos.m_positionY <= MECHANAR_ROOM_MIN_Y + 2.0f) ||
                           (botPos.m_positionY >= MECHANAR_ROOM_MAX_Y - 2.0f);
        
        // Stuck detection
        bool wasStuck = false;
        if (g_flame_lastMoveTime.count(botGuid) && 
            (currentTime - g_flame_lastMoveTime[botGuid]) > 1500 &&
            !g_flame_inSafePosition[botGuid])
        {
            wasStuck = true;
        }
        
        g_flame_lastMoveTime[botGuid] = currentTime;
        g_flame_inSafePosition[botGuid] = false;
        
        // Alternative movement for wall collision or stuck state
        if (nearWall || veryNearWall || wasStuck)
        {
            // Calculate escape direction away from flame
            float flameAngle = targetingFlame->GetAngle(bot);
            const Position centerPos = MECHANAR_SEPETHREA_CENTER;
            
            // Try positions in arc away from flame
            for (int attempt = 0; attempt < 8; ++attempt)
            {
                // Calculate escape angle with variation
                float angleVariation = (attempt - 4) * (M_PI / 12.0f);
                float escapeAngle = flameAngle + angleVariation;
                
                // Project escape position from room center
                float escapeDistance = 10.0f + (attempt * 1.5f);
                
                float x = centerPos.GetPositionX() + cos(escapeAngle) * escapeDistance;
                float y = centerPos.GetPositionY() + sin(escapeAngle) * escapeDistance;
                float z = bot->GetPositionZ();
                
                Position testPos(x, y, z, 0.0f);
                
                // Validate escape position
                if (IsPositionSafe(testPos) && 
                    testPos.GetExactDist2d(targetingFlame->GetPosition()) > 15.0f &&
                    testPos.GetExactDist2d(botPos) > 5.0f &&
                    !bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA))
                {
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                 MovementPriority::MOVEMENT_FORCED);
                }
            }
            
            // Fallback: move along east-west axis
            if (veryNearWall || wasStuck)
            {
                // Move east or west based on flame position
                float flameX = targetingFlame->GetPositionX();
                float botX = botPos.m_positionX;
                
                // Calculate east-west movement
                float targetX, targetY;
                if (flameX > botX)
                {
                    // Flame is east, move west
                    targetX = botX - 8.0f;
                }
                else
                {
                    // Flame is west, move east  
                    targetX = botX + 8.0f;
                }
                
                // Adjust Y position toward center if needed
                targetY = botPos.m_positionY;
                if (botPos.m_positionY < centerPos.GetPositionY() - 3.0f)
                    targetY += 2.0f;
                else if (botPos.m_positionY > centerPos.GetPositionY() + 3.0f)
                    targetY -= 2.0f;
                
                Position movePos(targetX, targetY, bot->GetPositionZ(), 0.0f);
                if (IsPositionSafe(movePos) && 
                    movePos.GetExactDist2d(targetingFlame->GetPosition()) > 12.0f)
                {
                    return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(),
                                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
        
        // Default flame kiting
        return FleePosition(targetingFlame->GetPosition(), 15.0f, 500U);
    }
    else if (flameDistance > 22.0f)
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

    // Stop spellcasting and movement
    if (bot->IsNonMeleeSpellCast(false))
    {
        if (botAI)
        {
            botAI->InterruptSpell();
        }
    }

    if (botAI)
    {
        botAI->Reset();
    }

    // Flee from fire trail
    return FleePosition(bot->GetPosition(), 12.0f, 200U);
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