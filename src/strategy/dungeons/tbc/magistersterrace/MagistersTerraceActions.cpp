#include "MagistersTerraceActions.h"
#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Spell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "AttackersValue.h"
#include "Playerbots.h"
#include "MoveSplineInit.h"

// Per-bot state maps for Kael'thas gravity lapse
std::map<ObjectGuid, uint32> g_kaelthas_lastMoveTime;
std::map<ObjectGuid, bool> g_kaelthas_inSafePosition;

// Kael'thas Actions
bool InterruptKaelthasPyroblastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PYROBLAST))
    {
        // Use interrupt spell list - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
            }
        }
    }
    return false;
}

bool InterruptKaelthasPyroblastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* pyroblastValue = botAI->GetAiObjectContext()->GetValue<bool>("kaelthas casting pyroblast");
    if (!pyroblastValue)
        return false;
    
    return pyroblastValue->Get();
}

bool AvoidGravityLapseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Check if boss is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // CRITICAL: Always interrupt any casting during gravity lapse
    if (bot->IsNonMeleeSpellCast(false))
    {
        bot->InterruptNonMeleeSpells(true);
        return true;
    }

    // FLIGHT STATE DETECTION - The key to survival
    bool hasFlightAura = bot->HasAura(SPELL_GRAVITY_LAPSE_FLY);
    bool hasDotAura = bot->HasAura(SPELL_GRAVITY_LAPSE_DOT);
    bool isInGravityLapse = hasFlightAura || hasDotAura;
    

    if (isInGravityLapse)
    {
        // FLYING PHASE: Smart positioning - move when needed, heal/DPS when safe
        
        // Flying movement constants
        static const float MIN_MOVEMENT_INTERVAL = 800U;   // Move every 0.8 seconds (faster)
        static const float MIN_FLIGHT_HEIGHT = 10.0f;      // Minimum height above ground
        static const float MAX_FLIGHT_HEIGHT = 20.0f;      // Maximum height (room ceiling)
        static const float FLIGHT_RADIUS = 28.0f;          // Stay near room edges
        static const float SAFE_SPHERE_DISTANCE = 15.0f;   // Safe distance from spheres
        
        // Get room center for circular flight pattern (Kael'thas boss room)
        Position roomCenter(148.5f, 187.0f, -16.6f); // Actual Kael'thas room center
        float distanceFromCenter = bot->GetDistance(roomCenter);
        float currentHeight = bot->GetPositionZ() - roomCenter.GetPositionZ();
        
        // Check for nearby dangerous spheres
        Unit* nearestSphere = nullptr;
        float sphereDistance = 50.0f;
        
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;
            
            if (unit->GetEntry() == NPC_ARCANE_SPHERE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < sphereDistance)
                {
                    nearestSphere = unit;
                    sphereDistance = distance;
                }
            }
        }
        
        // Determine if movement is urgently needed
        bool shouldMove = false;
        
        // EMERGENCY: Sphere too close - immediate movement required
        if (nearestSphere && sphereDistance < SAFE_SPHERE_DISTANCE)
        {
            shouldMove = true;
        }
        
        // EMERGENCY: Force movement if haven't moved recently (prevent landing)
        if ((currentTime - g_kaelthas_lastMoveTime[botGuid]) > MIN_MOVEMENT_INTERVAL)
        {
            shouldMove = true;
        }
        
        // EMERGENCY: If bot is too low, force immediate upward movement
        if (currentHeight < MIN_FLIGHT_HEIGHT)
        {
            shouldMove = true;
        }
        
        // EMERGENCY: If bot is on the ground (same Z as room), force immediate ascent
        if (abs(bot->GetPositionZ() - roomCenter.GetPositionZ()) < 2.0f)
        {
            shouldMove = true;
        }
        
        // EMERGENCY: If bot's movement is idle, force movement to prevent landing
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
        {
            shouldMove = true;
        }
        
        // Update safe position status
        g_kaelthas_inSafePosition[botGuid] = !shouldMove && (!nearestSphere || sphereDistance > SAFE_SPHERE_DISTANCE);
        
        if (shouldMove)
        {
            // SPHERE AVOIDANCE: Check for nearby Arcane Spheres first
            Unit* nearestSphere = nullptr;
            float sphereDistance = 50.0f;
            
            // Find closest Arcane Sphere using proven pattern
            const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto& npc : npcs)
            {
                Unit* unit = botAI->GetUnit(npc);
                if (!unit || !unit->IsAlive())
                    continue;
                
                if (unit->GetEntry() == NPC_ARCANE_SPHERE)
                {
                    float distance = bot->GetDistance(unit);
                    if (distance < sphereDistance)
                    {
                        nearestSphere = unit;
                        sphereDistance = distance;
                    }
                }
            }
            
            float newX, newY, newZ;
            bool useAvoidanceMovement = false;
            
            // SMART SPHERE AVOIDANCE: Move perpendicular to sphere direction to avoid walls
            if (nearestSphere && sphereDistance < 15.0f)
            {
                useAvoidanceMovement = true;
                
                // Calculate vector from room center to bot (for wall avoidance)
                float centerToBotX = bot->GetPositionX() - roomCenter.GetPositionX();
                float centerToBotY = bot->GetPositionY() - roomCenter.GetPositionY();
                float centerToBotDist = sqrt(centerToBotX * centerToBotX + centerToBotY * centerToBotY);
                
                // Calculate vector from sphere to bot
                float sphereToBotX = bot->GetPositionX() - nearestSphere->GetPositionX();
                float sphereToBotY = bot->GetPositionY() - nearestSphere->GetPositionY();
                float sphereToBotDist = sqrt(sphereToBotX * sphereToBotX + sphereToBotY * sphereToBotY);
                
                if (sphereToBotDist > 0.1f)
                {
                    sphereToBotX /= sphereToBotDist;
                    sphereToBotY /= sphereToBotDist;
                }
                
                // If bot is near wall (> 10 yards from center), move perpendicular instead of away
                if (centerToBotDist > 10.0f)
                {
                    // Move perpendicular to sphere direction (90 degrees)
                    newX = bot->GetPositionX() + (-sphereToBotY) * 8.0f; // Perpendicular vector
                    newY = bot->GetPositionY() + sphereToBotX * 8.0f;
                    
                    // If perpendicular movement goes further from center, try the other direction
                    float newCenterDist = sqrt((newX - roomCenter.GetPositionX()) * (newX - roomCenter.GetPositionX()) + 
                                             (newY - roomCenter.GetPositionY()) * (newY - roomCenter.GetPositionY()));
                    if (newCenterDist > centerToBotDist)
                    {
                        // Try opposite perpendicular direction
                        newX = bot->GetPositionX() + sphereToBotY * 8.0f;
                        newY = bot->GetPositionY() + (-sphereToBotX) * 8.0f;
                    }
                }
                else
                {
                    // Normal avoidance - move away from sphere
                    newX = bot->GetPositionX() + sphereToBotX * 8.0f;
                    newY = bot->GetPositionY() + sphereToBotY * 8.0f;
                }
                
                // Ensure we don't go too far from room center (wall collision prevention)
                float finalCenterDist = sqrt((newX - roomCenter.GetPositionX()) * (newX - roomCenter.GetPositionX()) + 
                                           (newY - roomCenter.GetPositionY()) * (newY - roomCenter.GetPositionY()));
                if (finalCenterDist > 12.0f) // Max 12 yards from center
                {
                    float scale = 12.0f / finalCenterDist;
                    newX = roomCenter.GetPositionX() + (newX - roomCenter.GetPositionX()) * scale;
                    newY = roomCenter.GetPositionY() + (newY - roomCenter.GetPositionY()) * scale;
                }
                
                newZ = bot->GetPositionZ(); // Keep same altitude
                
                // Clamp Z coordinate to reasonable flight limits
                float groundZ = roomCenter.GetPositionZ();
                newZ = std::max(groundZ + 8.0f, std::min(groundZ + 15.0f, newZ));
            }
            
            // GROUP COHESION MOVEMENT: Keep bots together for healing range
            if (!useAvoidanceMovement)
            {
                // EMERGENCY ASCENT: If too low, move straight up first
                if (currentHeight < MIN_FLIGHT_HEIGHT)
                {
                    newX = bot->GetPositionX();
                    newY = bot->GetPositionY();
                    newZ = roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT + 5.0f; // Force high altitude
                }
                else
                {
                    // GROUP FORMATION: Try to stay near other group members
                    Position groupCenter = roomCenter;
                    int groupMemberCount = 0;
                    float totalX = 0, totalY = 0, totalZ = 0;
                    
                    // Calculate average position of nearby group members
                    Group* group = bot->GetGroup();
                    if (group)
                    {
                        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                        {
                            Player* member = itr->GetSource();
                            if (member && member != bot && member->IsAlive() && 
                                member->HasAura(SPELL_GRAVITY_LAPSE_FLY) && 
                                bot->GetDistance(member) < 50.0f)
                            {
                                totalX += member->GetPositionX();
                                totalY += member->GetPositionY();
                                totalZ += member->GetPositionZ();
                                groupMemberCount++;
                            }
                        }
                    }
                    
                    if (groupMemberCount > 0)
                    {
                        // Move towards group center but not too close
                        groupCenter.Relocate(totalX / groupMemberCount, totalY / groupMemberCount, totalZ / groupMemberCount);
                        float distanceToGroup = bot->GetDistance(groupCenter);
                        
                        if (distanceToGroup > 25.0f) // Too far from group
                        {
                            // Move closer to group
                            float moveDistance = std::min(15.0f, distanceToGroup - 20.0f);
                            float angle = bot->GetAngle(&groupCenter);
                            newX = bot->GetPositionX() + cos(angle) * moveDistance;
                            newY = bot->GetPositionY() + sin(angle) * moveDistance;
                        }
                        else if (distanceToGroup < 10.0f) // Too close to group
                        {
                            // Move slightly away to avoid stacking
                            float angle = bot->GetAngle(&groupCenter) + M_PI; // Opposite direction
                            newX = bot->GetPositionX() + cos(angle) * 8.0f;
                            newY = bot->GetPositionY() + sin(angle) * 8.0f;
                        }
                        else
                        {
                            // Good distance, just do small circular movement around group
                            float currentAngle = atan2(bot->GetPositionY() - groupCenter.GetPositionY(), 
                                                     bot->GetPositionX() - groupCenter.GetPositionX());
                            float newAngle = currentAngle + (M_PI / 12.0f); // 15 degree increments (slower)
                            
                            newX = groupCenter.GetPositionX() + cos(newAngle) * 8.0f; // Very tight radius
                            newY = groupCenter.GetPositionY() + sin(newAngle) * 8.0f;
                        }
                        
                        newZ = groupCenter.GetPositionZ(); // Match group altitude
                    }
                    else
                    {
                        // No group members found, stay near boss with tight movement
                        float currentAngle = atan2(bot->GetPositionY() - roomCenter.GetPositionY(), 
                                                 bot->GetPositionX() - roomCenter.GetPositionX());
                        
                        float newAngle = currentAngle + (M_PI / 8.0f); // 22.5 degree increments
                        
                        newX = roomCenter.GetPositionX() + cos(newAngle) * 8.0f; // Very tight radius
                        newY = roomCenter.GetPositionY() + sin(newAngle) * 8.0f;
                        newZ = roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT + 5.0f;
                    }
                    
                    // WALL COLLISION PREVENTION: Ensure all movement stays within safe bounds
                    float distanceFromCenter = sqrt((newX - roomCenter.GetPositionX()) * (newX - roomCenter.GetPositionX()) + 
                                                   (newY - roomCenter.GetPositionY()) * (newY - roomCenter.GetPositionY()));
                    if (distanceFromCenter > 10.0f) // Max 10 yards from center to avoid walls
                    {
                        float scale = 10.0f / distanceFromCenter;
                        newX = roomCenter.GetPositionX() + (newX - roomCenter.GetPositionX()) * scale;
                        newY = roomCenter.GetPositionY() + (newY - roomCenter.GetPositionY()) * scale;
                    }
                    
                    // Clamp to safe flight limits
                    newZ = std::max(roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT, 
                                   std::min(roomCenter.GetPositionZ() + MAX_FLIGHT_HEIGHT, newZ));
                }
            }
            
            // CRITICAL: Use proper flying movement API to maintain altitude
            bot->GetMotionMaster()->Clear(); // Clear any conflicting movement generators
            bot->StopMoving(); // Stop current movement
            
            // Use MoveSplineInit for 3D flying movement that preserves altitude
            bool moved = false;
            if (bot->IsFlying() || bot->IsLevitating() || hasFlightAura)
            {
                Movement::MoveSplineInit init(bot);
                init.MoveTo(newX, newY, newZ, false, false);  // No pathfinding for flight
                init.SetFly();                                // KEY: Enables flying movement mode
                init.Launch();
                
                // Calculate movement tracking
                float distance = bot->GetExactDist(newX, newY, newZ);
                float delay = 1000.0f * distance / bot->GetSpeed(MOVE_FLIGHT);
                AI_VALUE(LastMovement&, "last movement").Set(bot->GetMapId(), newX, newY, newZ, bot->GetOrientation(), delay, MovementPriority::MOVEMENT_FORCED);
                moved = true;
            }
            else
            {
                // Fallback to ground movement if not flying
                moved = MoveTo(bot->GetMapId(), newX, newY, newZ, 
                              false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            
            if (moved)
            {
                g_kaelthas_lastMoveTime[botGuid] = currentTime;
                return true;
            }
            
            // Fallback: 3D random movement if primary patterns fail
            float randomAngle = frand(0, 2 * M_PI);
            float verticalOffset = frand(-4.0f, 4.0f); // Random vertical movement
            static const float FLIGHT_MOVEMENT_RADIUS = 15.0f; // Define missing constant
            
            float fallbackX = bot->GetPositionX() + cos(randomAngle) * FLIGHT_MOVEMENT_RADIUS;
            float fallbackY = bot->GetPositionY() + sin(randomAngle) * FLIGHT_MOVEMENT_RADIUS;
            float fallbackZ = roomCenter.GetPositionZ() + 8.0f + verticalOffset;
            
            // Clamp fallback Z to safe limits
            fallbackZ = std::max(roomCenter.GetPositionZ() + 3.0f, 
                               std::min(roomCenter.GetPositionZ() + 15.0f, fallbackZ));
            
            // FALLBACK: Also clear movement for fallback and force aerial positioning
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
            
            // Use flying movement API for fallback too
            if (bot->IsFlying() || bot->IsLevitating() || hasFlightAura)
            {
                Movement::MoveSplineInit init(bot);
                init.MoveTo(fallbackX, fallbackY, fallbackZ, false, false);
                init.SetFly();
                init.Launch();
                
                float distance = bot->GetExactDist(fallbackX, fallbackY, fallbackZ);
                float delay = 1000.0f * distance / bot->GetSpeed(MOVE_FLIGHT);
                AI_VALUE(LastMovement&, "last movement").Set(bot->GetMapId(), fallbackX, fallbackY, fallbackZ, bot->GetOrientation(), delay, MovementPriority::MOVEMENT_FORCED);
                moved = true;
            }
            else
            {
                moved = MoveTo(bot->GetMapId(), fallbackX, fallbackY, fallbackZ,
                              false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            
            if (moved)
            {
                g_kaelthas_lastMoveTime[botGuid] = currentTime;
                return true;
            }
        }
        
        // CONDITIONAL ACTION BLOCKING: Only block actions when spheres are close
        if (nearestSphere && sphereDistance < SAFE_SPHERE_DISTANCE)
        {
            // Emergency sphere avoidance - block other actions
            if (bot->IsInCombat())
            {
                bot->AttackStop(); // Stop attacking
            }
            bot->SetTarget(ObjectGuid::Empty); // Clear target
            return true; // Block other actions - emergency movement
        }
        else
        {
            // Normal movement during gravity lapse - don't block healing/DPS
            return false; // Allow healing and DPS while moving
        }
    }
    else
    {
        // POST-FLIGHT PHASE: Gravity lapse has ended, force bots back to ground
        static std::map<ObjectGuid, bool> wasFlying;
        
        // Check if bot was flying but gravity lapse is now over
        if (wasFlying[botGuid] && !hasFlightAura && !hasDotAura)
        {
            // Force bot back to ground level for normal combat
            Position groundPos = boss->GetPosition();
            float groundZ = groundPos.GetPositionZ();
            
            // Move to ground level near the boss
            bool moved = MoveTo(bot->GetMapId(), groundPos.GetPositionX() + frand(-10.0f, 10.0f), 
                               groundPos.GetPositionY() + frand(-10.0f, 10.0f), groundZ,
                               false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            
            if (moved)
            {
                wasFlying[botGuid] = false;
                return true;
            }
        }
        
        // Track flying state
        wasFlying[botGuid] = hasFlightAura || hasDotAura;
        
        // PRE-FLIGHT PHASE: Initial gravity lapse cast detected
        Position centerPos = boss->GetPosition();
        float distance = bot->GetDistance(centerPos);
        
        // Move away from center in preparation for flight phase
        if (distance < 20.0f)
        {
            // Simple flee to room edges before flight starts
            if (FleePosition(centerPos, 25.0f, 500U))
            {
                g_kaelthas_lastMoveTime[botGuid] = currentTime;
                return true;
            }
        }
    }

    return false;
}

bool AvoidGravityLapseAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // SIMPLIFIED: Only be useful when spheres are actually dangerous
    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if in gravity lapse
    bool hasFlightAura = bot->HasAura(SPELL_GRAVITY_LAPSE_FLY);
    bool hasDotAura = bot->HasAura(SPELL_GRAVITY_LAPSE_DOT);
    
    if (!hasFlightAura && !hasDotAura)
        return false;
    
    // Only be useful when there's a sphere nearby (< 12 yards)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;
        
        if (unit->GetEntry() == NPC_ARCANE_SPHERE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 12.0f) // Only when really close
            {
                return true; // Emergency movement needed
            }
        }
    }
    
    // No immediate sphere threat - allow healing/DPS
    return false;
}

bool FleeArcaneSphereAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find nearest arcane sphere using proven pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestSphere = nullptr;
    float closestDistance = 15.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ARCANE_SPHERE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestSphere = unit;
                closestDistance = distance;
            }
        }
    }

    if (closestSphere && closestDistance < 10.0f)
    {
        return FleePosition(closestSphere->GetPosition(), 15.0f, 500U);
    }

    return false;
}

bool FleeArcaneSphereAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* sphereValue = botAI->GetAiObjectContext()->GetValue<bool>("kaelthas arcane sphere nearby");
    if (!sphereValue)
        return false;
    
    return sphereValue->Get();
}

bool AvoidFlamestrikeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find nearest flamestrike trigger within dangerous range
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestTrigger = nullptr;
    float closestDistance = 12.0f; // 12 yard danger radius

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FLAMESTRIKE_TRIGGER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestTrigger = unit;
                closestDistance = distance;
            }
        }
    }

    if (closestTrigger)
    {
        // Move away from flamestrike trigger position
        Position triggerPos = closestTrigger->GetPosition();
        return FleePosition(triggerPos, 15.0f, 1000U); // Flee 15 yards away quickly
    }

    return false;
}

bool AvoidFlamestrikeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // CRITICAL FIX: Use direct trigger logic instead of Value system
    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for flamestrike trigger creatures within dangerous range
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FLAMESTRIKE_TRIGGER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 12.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

// Vexallus Actions
bool AttackPureEnergyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // ENHANCED PURE ENERGY TARGETING: Multi-method approach
    Unit* pureEnergy = nullptr;
    float closestDistance = 80.0f; // Increased detection range

    // Method 1: Check hostile NPCs list first (most reliable)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                pureEnergy = unit;
                closestDistance = distance;
            }
        }
    }

    // Method 2: Direct creature search if no Pure Energy found in hostile list
    if (!pureEnergy)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 80.0f); // Wide search
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 80.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_PURE_ENERGY)
            {
                float distance = bot->GetDistance(unit);
                if (distance < closestDistance)
                {
                    pureEnergy = unit;
                    closestDistance = distance;
                }
            }
        }
    }

    // Method 3: Emergency fallback - find ANY Pure Energy if none found above
    if (!pureEnergy)
    {
        std::list<Unit*> allTargets;
        Acore::AnyUnitInObjectRangeCheck u_check_all(bot, 100.0f); // Maximum search range
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher_all(bot, allTargets, u_check_all);
        Cell::VisitObjects(bot, searcher_all, 100.0f);

        for (std::list<Unit*>::iterator i = allTargets.begin(); i != allTargets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            // Accept ANY Pure Energy as emergency target
            if (unit->GetEntry() == NPC_PURE_ENERGY)
            {
                float distance = bot->GetDistance(unit);
                if (distance < closestDistance)
                {
                    pureEnergy = unit;
                    closestDistance = distance;
                }
            }
        }
    }

    if (pureEnergy)
    {
        return Attack(pureEnergy);
    }

    return false;
}

bool AttackPureEnergyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // CRITICAL FIX: Use direct trigger logic instead of Value system
    // The Value system may be failing to map trigger values properly
    
    // Check if Vexallus is in combat first
    Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check hostile NPCs list for Pure Energy (most reliable method)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY)
        {
            return true;
        }
    }
    
    return false;
}

// Selin Fireheart Actions
bool AvoidFelExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "selin fireheart");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Move away during Fel Explosion cast
    float distance = bot->GetDistance(boss);
    if (distance < 15.0f)
    {
        return FleePosition(boss->GetPosition(), 20.0f, 500U);
    }

    return false;
}

bool AvoidFelExplosionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* explosionValue = botAI->GetAiObjectContext()->GetValue<bool>("selin fireheart fel explosion");
    if (!explosionValue)
        return false;
    
    return explosionValue->Get();
}

bool AttackFelCrystalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Selin is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "selin fireheart");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // ENHANCED CRYSTAL DETECTION: Use the same multi-method approach as the trigger
    Unit* crystal = nullptr;
    float closestDistance = 60.0f; // Increased detection range

    // Method 1: Check hostile NPCs list first
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FEL_CRYSTAL)
        {
            // EXPANDED DETECTION: More comprehensive crystal state checking
            bool isSelectable = !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
            bool isCasting = unit->HasUnitState(UNIT_STATE_CASTING) || unit->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
            bool isChanneling = unit->HasAura(SPELL_MANA_RAGE) || boss->HasAura(SPELL_MANA_RAGE);
            bool inCombat = unit->IsInCombat();
            bool hasTarget = unit->GetVictim() != nullptr;
            
            // Crystal is active if ANY of these conditions are met
            if (isSelectable || isCasting || isChanneling || inCombat || hasTarget)
            {
                float distance = bot->GetDistance(unit);
                if (distance < closestDistance)
                {
                    crystal = unit;
                    closestDistance = distance;
                }
            }
        }
    }

    // Method 2: Direct creature search if no crystal found in hostile list
    if (!crystal)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f); // Increased range
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 60.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_FEL_CRYSTAL)
            {
                // Same comprehensive detection as Method 1
                bool isSelectable = !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                bool isCasting = unit->HasUnitState(UNIT_STATE_CASTING) || unit->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
                bool isChanneling = unit->HasAura(SPELL_MANA_RAGE) || boss->HasAura(SPELL_MANA_RAGE);
                bool inCombat = unit->IsInCombat();
                bool hasTarget = unit->GetVictim() != nullptr;
                
                if (isSelectable || isCasting || isChanneling || inCombat || hasTarget)
                {
                    float distance = bot->GetDistance(unit);
                    if (distance < closestDistance)
                    {
                        crystal = unit;
                        closestDistance = distance;
                    }
                }
            }
        }
    }

    // Method 3: If boss is channeling but no crystal found, find ANY crystal nearby
    if (!crystal && (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE)))
    {
        std::list<Unit*> allTargets;
        Acore::AnyUnitInObjectRangeCheck u_check_all(bot, 80.0f); // Even wider search
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher_all(bot, allTargets, u_check_all);
        Cell::VisitObjects(bot, searcher_all, 80.0f);

        for (std::list<Unit*>::iterator i = allTargets.begin(); i != allTargets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            // Accept ANY crystal if boss is channeling (emergency fallback)
            if (unit->GetEntry() == NPC_FEL_CRYSTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < closestDistance)
                {
                    crystal = unit;
                    closestDistance = distance;
                }
            }
        }
    }

    if (crystal)
    {
        return Attack(crystal);
    }

    return false;
}

bool AttackFelCrystalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* crystalValue = botAI->GetAiObjectContext()->GetValue<bool>("fel crystal nearby");
    if (!crystalValue)
        return false;
    
    return crystalValue->Get();
}

// Delrissa Actions
bool AttackDelrissaAddAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "priestess delrissa");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // PRIORITY-BASED targeting system (highest to lowest threat)
    // Based on boss script research and helper abilities
    struct HelperPriority {
        uint32 npcId;
        uint32 priority;
        const char* name;
    };

    const HelperPriority delrissaHelpers[] = {
        // PRIORITY 1: Healers (must die first - heal others including Delrissa)
        {24554, 100, "Eramas Brightblaze"},      // Mage - Polymorph, Fireball (high threat caster)
        {24558, 95,  "Elris Duskhallow"},       // Warlock - Fear, Shadow Bolt (high threat caster)
        
        // PRIORITY 2: Dangerous Casters (crowd control and damage)
        {24561, 85,  "Yazzaj"},                 // Warlock - Fear, Shadow damage
        {24553, 80,  "Apoko"},                  // Mage - Similar to Eramas
        
        // PRIORITY 3: Melee DPS with special abilities
        {24557, 70,  "Kagani Nightstrike"},    // Rogue - Gouge, Kidney Shot (stuns)
        {24559, 65,  "Warlord Salaris"},       // Warrior - Intercept, Mortal Strike
        
        // PRIORITY 4: Lower threat melee
        {24555, 50,  "Garaxxas"},              // Melee DPS
        {24556, 45,  "Zelfan"},                // Melee DPS
    };

    // Find highest priority target that's alive and in combat
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityTarget = nullptr;
    uint32 highestPriority = 0;
    float targetDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || !unit->IsInCombat())
            continue;

        if (!AttackersValue::IsValidTarget(unit, bot))
            continue;

        // Check if this unit is one of Delrissa's helpers
        for (const auto& helper : delrissaHelpers)
        {
            if (unit->GetEntry() == helper.npcId)
            {
                // Priority system: Higher priority value = more important target
                // Also consider distance for ties (closer target preferred)
                float distance = bot->GetDistance(unit);
                bool isBetterTarget = false;

                if (helper.priority > highestPriority)
                {
                    // Higher priority always wins
                    isBetterTarget = true;
                }
                else if (helper.priority == highestPriority && distance < targetDistance)
                {
                    // Same priority, prefer closer target
                    isBetterTarget = true;
                }

                if (isBetterTarget)
                {
                    priorityTarget = unit;
                    highestPriority = helper.priority;
                    targetDistance = distance;
                }
                break;
            }
        }
    }

    // Fallback: If no priority targets found, try direct search
    if (!priorityTarget)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 50.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive() || !unit->IsInCombat())
                continue;

            if (!AttackersValue::IsValidTarget(unit, bot))
                continue;

            for (const auto& helper : delrissaHelpers)
            {
                if (unit->GetEntry() == helper.npcId)
                {
                    float distance = bot->GetDistance(unit);
                    if (helper.priority > highestPriority || 
                        (helper.priority == highestPriority && distance < targetDistance))
                    {
                        priorityTarget = unit;
                        highestPriority = helper.priority;
                        targetDistance = distance;
                    }
                    break;
                }
            }
        }
    }

    if (priorityTarget)
    {
        return Attack(priorityTarget);
    }

    return false;
}

bool AttackDelrissaAddAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* addActiveValue = botAI->GetAiObjectContext()->GetValue<bool>("delrissa add active");
    if (!addActiveValue)
        return false;
    
    return addActiveValue->Get();
}

// Interrupt dangerous helper abilities
bool InterruptDelrissaHelperAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "priestess delrissa");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // High priority spells to interrupt from helpers
    const uint32 dangerousSpells[] = {
        33831,  // Polymorph (Eramas)
        5782,   // Fear (Elris/Yazzaj)
        34447,  // Arcane Missiles
        32364,  // Unstable Affliction
        36679,  // Mortal Strike
        44164,  // Heal spells
    };

    // Find helpers casting dangerous abilities
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* interruptTarget = nullptr;
    float closestDistance = 30.0f; // Interrupt range

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || !unit->IsInCombat())
            continue;

        // Check if it's a Delrissa helper
        const uint32 delrissaHelpers[] = {24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556};
        bool isHelper = false;
        for (uint32 helperId : delrissaHelpers)
        {
            if (unit->GetEntry() == helperId)
            {
                isHelper = true;
                break;
            }
        }

        if (!isHelper)
            continue;

        // Check if casting dangerous spell
        if (unit->HasUnitState(UNIT_STATE_CASTING))
        {
            for (uint32 spellId : dangerousSpells)
            {
                if (unit->FindCurrentSpellBySpellId(spellId))
                {
                    float distance = bot->GetDistance(unit);
                    if (distance < closestDistance)
                    {
                        interruptTarget = unit;
                        closestDistance = distance;
                        break;
                    }
                }
            }
        }
    }

    if (interruptTarget)
    {
        // Use interrupt spell list - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, interruptTarget, false))
                {
                    return botAI->CastSpell(spellId, interruptTarget);
                }
            }
        }
    }

    return false;
}

bool InterruptDelrissaHelperAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Only useful if we have interrupt abilities
    Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
    if (!spellIdsValue)
        return false;

    std::list<uint32> spellIds = spellIdsValue->Get();
    return !spellIds.empty();
}

// Handle being dispelled by Delrissa
bool DelrissaDispelHandlingAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "priestess delrissa");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Re-apply important buffs that might have been dispelled
    const uint32 importantBuffs[] = {
        25570,  // Blessing of Might
        25572,  // Blessing of Wisdom
        23028,  // Arcane Intellect
        1126,   // Mark of the Wild
        27681,  // Prayer of Spirit
        15473,  // Shadowform (priests)
    };

    for (uint32 buffId : importantBuffs)
    {
        if (!bot->HasAura(buffId) && botAI->CanCastSpell(buffId, bot, false))
        {
            return botAI->CastSpell(buffId, bot);
        }
    }

    // If tank, ensure defensive stance/righteous fury
    if (botAI->IsTank(bot))
    {
        const uint32 tankStances[] = {
            71,     // Defensive Stance
            5487,   // Bear Form  
            25780,  // Righteous Fury
        };

        for (uint32 stanceId : tankStances)
        {
            if (!bot->HasAura(stanceId) && botAI->CanCastSpell(stanceId, bot, false))
            {
                return botAI->CastSpell(stanceId, bot);
            }
        }
    }

    return false;
}

bool DelrissaDispelHandlingAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Check if Delrissa encounter is active
    Unit* boss = AI_VALUE2(Unit*, "find target", "priestess delrissa");
    return boss && boss->IsAlive() && boss->IsInCombat();
}