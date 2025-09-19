#include "MagistersTerraceActions.h"
#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Spell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectAccessor.h"
#include "AttackersValue.h"
#include "Playerbots.h"
#include "MoveSplineInit.h"
#include "Group.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float PI_F = 3.14159265358979323846f;
}

// Per-bot state maps for Kael'thas gravity lapse
std::map<ObjectGuid, uint32> g_kaelthas_lastMoveTime;
std::map<ObjectGuid, bool> g_kaelthas_inSafePosition;
std::map<ObjectGuid, Position> g_kaelthas_lastFlightPosition;
std::map<ObjectGuid, uint32> g_kaelthas_stuckSince;
std::map<ObjectGuid, float> g_kaelthas_orbitAngle;
std::map<ObjectGuid, float> g_kaelthas_orbitRadius;

// Per-bot state maps for Delrissa add targeting stability
std::map<ObjectGuid, ObjectGuid> g_delrissa_lockedTarget;
std::map<ObjectGuid, uint32> g_delrissa_lockTime;

// Per-bot state maps for Selin crystal focus
std::map<ObjectGuid, ObjectGuid> g_selin_crystalLock;
std::map<ObjectGuid, uint32> g_selin_crystalLockTime;
Unit* MagistersTerraceHelpers::SelectActiveFelCrystal(Player* bot, PlayerbotAI* botAI, Unit* boss)
{
    if (!bot || !botAI || !boss)
        return nullptr;

    Unit* activeCrystal = nullptr;
    float bestDistance = 80.0f;

    auto considerUnit = [&](Unit* unit)
    {
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_FEL_CRYSTAL)
            return;

        bool selectable = !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
        bool castingManaRage = unit->HasUnitState(UNIT_STATE_CASTING) || unit->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
        bool targetingBoss = unit->GetVictim() == boss;

        if (!selectable && !castingManaRage && !targetingBoss)
            return;

        float distance = bot->GetDistance(unit);
        if (!activeCrystal || distance < bestDistance)
        {
            activeCrystal = unit;
            bestDistance = distance;
        }
    };

    if (Aura* manaRage = boss->GetAura(SPELL_MANA_RAGE))
    {
        ObjectGuid casterGuid = manaRage->GetCasterGUID();
        if (casterGuid)
        {
            Unit* caster = botAI->GetUnit(casterGuid);
            if (!caster)
                caster = ObjectAccessor::GetUnit(*bot, casterGuid);

            considerUnit(caster);
            if (activeCrystal)
                return activeCrystal;
        }
    }

    if (AiObjectContext* context = botAI->GetAiObjectContext())
    {
        if (auto* npcsValue = context->GetValue<GuidVector>("nearest hostile npcs"))
        {
            GuidVector npcs = npcsValue->Get();
            for (ObjectGuid const& guid : npcs)
                considerUnit(botAI->GetUnit(guid));
        }

        if (!activeCrystal)
        {
            if (auto* possibleValue = context->GetValue<GuidVector>("possible targets"))
            {
                GuidVector possible = possibleValue->Get();
                for (ObjectGuid const& guid : possible)
                    considerUnit(botAI->GetUnit(guid));
            }
        }
    }

    if (!activeCrystal)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 80.0f);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 80.0f);

        for (Unit* unit : targets)
            considerUnit(unit);
    }

    return activeCrystal;
}

namespace
{
    struct DelrissaHelperPriority
    {
        uint32 entry;
        uint32 priority;
    };

    static const DelrissaHelperPriority kDelrissaHelperPriority[] =
    {
        {24554, 100}, // Eramas Brightblaze
        {24558, 95},  // Elris Duskhallow
        {24561, 85},  // Yazzaj
        {24553, 80},  // Apoko
        {24557, 70},  // Kagani Nightstrike
        {24559, 65},  // Warlord Salaris
        {24555, 50},  // Garaxxas
        {24556, 45}   // Zelfan
    };

    Unit* SelectDelrissaHelperTarget(Player* bot, PlayerbotAI* botAI, GuidVector const& candidates, bool requireCombatCheck)
    {
        Unit* bestTarget = nullptr;
        uint32 bestPriority = 0;
        float bestDistance = 50.0f;

        for (ObjectGuid const& guid : candidates)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            if (requireCombatCheck && !unit->IsInCombat())
                continue;

            if (!AttackersValue::IsValidTarget(unit, bot))
                continue;

            for (DelrissaHelperPriority const& helper : kDelrissaHelperPriority)
            {
                if (unit->GetEntry() != helper.entry)
                    continue;

                float distance = bot->GetDistance(unit);
                if (helper.priority > bestPriority || (helper.priority == bestPriority && distance < bestDistance))
                {
                    bestTarget = unit;
                    bestPriority = helper.priority;
                    bestDistance = distance;
                }
                break;
            }
        }

        return bestTarget;
    }
}
// Per-bot state maps for Pure Energy timeout mechanism
std::map<ObjectGuid, uint32> g_pureEnergy_lastSeenTime;
std::map<ObjectGuid, uint32> g_pureEnergy_stuckTime;
std::map<ObjectGuid, ObjectGuid> g_pureEnergy_lastTarget;

// Kael'thas Actions
bool InterruptKaelthasPyroblastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
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
    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
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
        static const float STUCK_MOVEMENT_EPSILON = 1.2f;
        static const uint32 STUCK_TIME_THRESHOLD = 900;

        Position currentPos;
        currentPos.Relocate(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());

        bool isStuck = false;

        auto lastPosIt = g_kaelthas_lastFlightPosition.find(botGuid);
        if (lastPosIt == g_kaelthas_lastFlightPosition.end())
        {
            g_kaelthas_lastFlightPosition[botGuid] = currentPos;
            g_kaelthas_stuckSince[botGuid] = currentTime;
        }
        else
        {
            float movedDistance = currentPos.GetExactDist2d(&lastPosIt->second);
            auto stuckIt = g_kaelthas_stuckSince.find(botGuid);
            if (stuckIt == g_kaelthas_stuckSince.end())
                stuckIt = g_kaelthas_stuckSince.emplace(botGuid, currentTime).first;

            if (movedDistance > STUCK_MOVEMENT_EPSILON)
            {
                stuckIt->second = currentTime;
            }
            else if ((currentTime - stuckIt->second) > STUCK_TIME_THRESHOLD)
            {
                isStuck = true;
            }

            lastPosIt->second = currentPos;
        }

        // FLYING PHASE: Smart positioning - move when needed, heal/DPS when safe
        
        // Flying movement constants
        static const float MIN_MOVEMENT_INTERVAL = 400U;   // Move every 0.4 seconds (FASTER)
        static const float MIN_FLIGHT_HEIGHT = 12.0f;      // Higher minimum height above ground
        static const float MAX_FLIGHT_HEIGHT = 22.0f;      // Slightly higher ceiling use
        static const float KITING_RADIUS = 21.0f;          // Wider circular pattern
        static const float MAX_ROOM_RADIUS = 26.0f;        // Allow full room coverage
        static const float SAFE_SPHERE_DISTANCE = 18.0f;   // Safe distance from spheres
        
        // Get room center for circular flight pattern (Kael'thas boss room)
        Position roomCenter(148.5f, 187.0f, -16.6f); // Actual Kael'thas room center
        float distanceFromCenter = bot->GetDistance(roomCenter);
        float currentHeight = bot->GetPositionZ() - roomCenter.GetPositionZ();
        bool nearRoomBoundary = distanceFromCenter > 16.0f;
        bool blockedFromCenter = !bot->IsWithinLOS(roomCenter.GetPositionX(), roomCenter.GetPositionY(), roomCenter.GetPositionZ() + 2.0f);
        
        // CRITICAL: IMMEDIATE EMERGENCY ASCENT when gravity lapse first detected
        // This prevents initial fall that gets bots killed by spheres
        if (currentHeight < MIN_FLIGHT_HEIGHT || abs(bot->GetPositionZ() - roomCenter.GetPositionZ()) < 3.0f)
        {
            // EMERGENCY ASCENT: Force immediate upward movement
            float emergencyX = bot->GetPositionX();
            float emergencyY = bot->GetPositionY(); 
            float emergencyZ = roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT + 8.0f; // HIGH altitude immediately
            
            // Clear any movement to prevent conflicts
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
            
            // Force flying movement to high altitude
            Movement::MoveSplineInit init(bot);
            init.MoveTo(emergencyX, emergencyY, emergencyZ, false, false);
            init.SetFly();
            init.Launch();
            
            // Track movement
            float distance = bot->GetExactDist(emergencyX, emergencyY, emergencyZ);
            float delay = 1000.0f * distance / bot->GetSpeed(MOVE_FLIGHT);
            AI_VALUE(LastMovement&, "last movement").Set(bot->GetMapId(), emergencyX, emergencyY, emergencyZ, bot->GetOrientation(), delay, MovementPriority::MOVEMENT_FORCED);
            
            g_kaelthas_lastMoveTime[botGuid] = currentTime;
            return true; // Emergency ascent complete
        }
        
        // Check for nearby dangerous spheres (single pass)
        Unit* nearestSphere = nullptr;
        float sphereDistance = 50.0f;
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;
            if (unit->GetEntry() != NPC_ARCANE_SPHERE)
                continue;
            float distance = bot->GetDistance(unit);
            if (distance < sphereDistance)
            {
                nearestSphere = unit;
                sphereDistance = distance;
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
        if (abs(bot->GetPositionZ() - roomCenter.GetPositionZ()) < 3.0f)
        {
            shouldMove = true;
        }
        
        // EMERGENCY: If bot's movement is idle, force movement to prevent landing
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
        {
            shouldMove = true;
        }

        if (isStuck)
        {
            shouldMove = true;
        }

        if (nearestSphere && nearRoomBoundary)
        {
            shouldMove = true;
        }

        if (nearestSphere && blockedFromCenter)
        {
            shouldMove = true;
        }

        // Update safe position status
        g_kaelthas_inSafePosition[botGuid] = !shouldMove && (!nearestSphere || sphereDistance > SAFE_SPHERE_DISTANCE);
        
        if (shouldMove)
        {
            static const float MIN_ORBIT_RADIUS = 14.0f;
            static const float ANGLE_STEP = PI_F / 10.0f; // ~18 degrees per move

            float& orbitAngle = g_kaelthas_orbitAngle[botGuid];
            float& orbitRadius = g_kaelthas_orbitRadius[botGuid];

            if (orbitRadius < MIN_ORBIT_RADIUS * 0.5f)
            {
                orbitAngle = atan2(bot->GetPositionY() - roomCenter.GetPositionY(),
                                   bot->GetPositionX() - roomCenter.GetPositionX());
                orbitRadius = std::clamp(distanceFromCenter, MIN_ORBIT_RADIUS, MAX_ROOM_RADIUS - 0.5f);
            }

            auto normalizeAngle = [](float angle) -> float
            {
                while (angle > PI_F)
                    angle -= 2.0f * PI_F;
                while (angle < -PI_F)
                    angle += 2.0f * PI_F;
                return angle;
            };

            auto estimateSphereDistance = [&](float angle, float radius) -> float
            {
                if (!nearestSphere)
                    return radius;
                float px = roomCenter.GetPositionX() + std::cos(angle) * radius;
                float py = roomCenter.GetPositionY() + std::sin(angle) * radius;
                return std::hypot(px - nearestSphere->GetPositionX(), py - nearestSphere->GetPositionY());
            };

            float desiredAngle = orbitAngle + ANGLE_STEP;
            float desiredRadius = std::clamp(orbitRadius, MIN_ORBIT_RADIUS, MAX_ROOM_RADIUS - 0.5f);

            if (nearestSphere)
            {
                float forwardDist = estimateSphereDistance(normalizeAngle(orbitAngle + ANGLE_STEP), desiredRadius);
                float backwardDist = estimateSphereDistance(normalizeAngle(orbitAngle - ANGLE_STEP), desiredRadius);
                desiredAngle = (backwardDist > forwardDist) ? orbitAngle - ANGLE_STEP : orbitAngle + ANGLE_STEP;

                if (sphereDistance < SAFE_SPHERE_DISTANCE)
                {
                    desiredRadius = std::clamp(orbitRadius + (SAFE_SPHERE_DISTANCE - sphereDistance) + 2.0f,
                                               MIN_ORBIT_RADIUS, MAX_ROOM_RADIUS - 0.5f);
                }
                else if (sphereDistance > SAFE_SPHERE_DISTANCE + 5.0f)
                {
                    desiredRadius = std::clamp(orbitRadius - 1.0f, MIN_ORBIT_RADIUS, MAX_ROOM_RADIUS - 0.5f);
                }
            }
            else
            {
                float targetRadius = KITING_RADIUS;
                if (desiredRadius < targetRadius)
                    desiredRadius = std::min(targetRadius, desiredRadius + 1.0f);
                else if (desiredRadius > targetRadius)
                    desiredRadius = std::max(targetRadius, desiredRadius - 1.0f);
            }

            Position desiredDestination;
            bool destinationFound = false;
            const float altitudeBase = roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT + 5.0f;
            const float minAltitude = roomCenter.GetPositionZ() + MIN_FLIGHT_HEIGHT;
            const float maxAltitude = roomCenter.GetPositionZ() + MAX_FLIGHT_HEIGHT;

            auto makeDestination = [&](float angle, float radius) -> Position
            {
                float x = roomCenter.GetPositionX() + std::cos(angle) * radius;
                float y = roomCenter.GetPositionY() + std::sin(angle) * radius;
                Position pos;
                pos.Relocate(x, y, std::clamp(altitudeBase, minAltitude, maxAltitude));
                return pos;
            };

            for (uint8 attempt = 0; attempt < 8 && !destinationFound; ++attempt)
            {
                float angleOffset = (attempt == 0 ? 0.0f : ((attempt % 2 == 0 ? 1.0f : -1.0f) *
                                  (static_cast<float>((attempt + 1) / 2) * (ANGLE_STEP / 2.0f))));
                float testAngle = normalizeAngle(desiredAngle + angleOffset);
                float testRadius = desiredRadius;

                if (nearestSphere)
                {
                    float predictedSphereDist = estimateSphereDistance(testAngle, testRadius);
                    if (predictedSphereDist < SAFE_SPHERE_DISTANCE - 2.0f)
                        continue;
                }

                Position candidate = makeDestination(testAngle, testRadius);
                if (!bot->IsWithinLOS(candidate.GetPositionX(), candidate.GetPositionY(), candidate.GetPositionZ()))
                    continue;

                desiredDestination = candidate;
                orbitAngle = testAngle;
                orbitRadius = testRadius;
                destinationFound = true;
            }

            if (!destinationFound)
            {
                float randomAngle = frand(0.0f, 2.0f * PI_F);
                float randomRadius = std::clamp(KITING_RADIUS, MIN_ORBIT_RADIUS, MAX_ROOM_RADIUS - 0.5f);
                desiredDestination = makeDestination(randomAngle, randomRadius);
                orbitAngle = normalizeAngle(randomAngle);
                orbitRadius = randomRadius;
            }

            auto attemptFlightMove = [&](const Position& target) -> bool
            {
                Position copy = target;
                copy.Relocate(target.GetPositionX(), target.GetPositionY(),
                    std::clamp(target.GetPositionZ(), minAltitude, maxAltitude));

                bot->GetMotionMaster()->Clear();
                bot->StopMoving();

                if (bot->IsFlying() || bot->IsLevitating() || hasFlightAura)
                {
                    Movement::MoveSplineInit init(bot);
                    init.MoveTo(copy.GetPositionX(), copy.GetPositionY(), copy.GetPositionZ(), false, false);
                    init.SetFly();
                    init.Launch();

                    float distance = bot->GetExactDist(copy.GetPositionX(), copy.GetPositionY(), copy.GetPositionZ());
                    float speed = bot->GetSpeed(MOVE_FLIGHT);
                    if (speed <= 0.0f)
                        speed = bot->GetSpeed(MOVE_RUN);
                    float delay = speed > 0.0f ? 1000.0f * (distance / speed) : 0.0f;
                    delay = std::min(delay, static_cast<float>(sPlayerbotAIConfig->maxWaitForMove));
                    AI_VALUE(LastMovement&, "last movement").Set(bot->GetMapId(), copy.GetPositionX(), copy.GetPositionY(), copy.GetPositionZ(),
                                                                 bot->GetOrientation(), delay, MovementPriority::MOVEMENT_FORCED);
                    g_kaelthas_lastMoveTime[botGuid] = currentTime;
                    return true;
                }

                if (MoveTo(bot->GetMapId(), copy.GetPositionX(), copy.GetPositionY(), copy.GetPositionZ(),
                           false, false, false, true, MovementPriority::MOVEMENT_FORCED))
                {
                    g_kaelthas_lastMoveTime[botGuid] = currentTime;
                    return true;
                }

                return false;
            };

            bool moved = attemptFlightMove(desiredDestination);

            if (!moved)
            {
                Position fallbackDestination = makeDestination(frand(0.0f, 2.0f * PI_F), MAX_ROOM_RADIUS - 1.0f);
                moved = attemptFlightMove(fallbackDestination);
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
        g_kaelthas_lastFlightPosition.erase(botGuid);
        g_kaelthas_stuckSince.erase(botGuid);
        g_kaelthas_orbitAngle.erase(botGuid);
        g_kaelthas_orbitRadius.erase(botGuid);
        // POST-FLIGHT PHASE: Gravity lapse has ended, force bots back to ground
        static std::map<ObjectGuid, bool> wasFlying;
        
        // Track flying state FIRST to capture transitions
        bool currentlyFlying = hasFlightAura || hasDotAura;
        bool wasAlreadyFlying = wasFlying[botGuid];
        wasFlying[botGuid] = currentlyFlying;
        
        // Check if bot WAS flying but gravity lapse is now over
        if (wasAlreadyFlying && !currentlyFlying)
        {
            // CRITICAL: Force bot back to ground level for normal combat
            Position groundPos = boss->GetPosition();
            float groundZ = groundPos.GetPositionZ();
            
            // Clear any aerial movement first
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
            
            // Move to ground level near the boss with ground-based movement
            bool moved = MoveTo(bot->GetMapId(), groundPos.GetPositionX() + frand(-8.0f, 8.0f), 
                               groundPos.GetPositionY() + frand(-8.0f, 8.0f), groundZ,
                               false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            
            if (moved)
            {
                return true; // Ground descent in progress
            }
        }
        
        // Also check if bot is still at high altitude after gravity lapse ends
        Position roomCenter(148.5f, 187.0f, -16.6f);
        float currentHeight = bot->GetPositionZ() - roomCenter.GetPositionZ();
        
        if (!currentlyFlying && currentHeight > 5.0f) // Bot is high but not flying
        {
            // Force descent to ground level
            Position groundPos = boss->GetPosition();
            bool moved = MoveTo(bot->GetMapId(), groundPos.GetPositionX() + frand(-8.0f, 8.0f), 
                               groundPos.GetPositionY() + frand(-8.0f, 8.0f), groundPos.GetPositionZ(),
                               false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            
            if (moved)
            {
                return true; // Forced descent
            }
        }
        
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
    
    return true;
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

    // CRITICAL: HEALERS NEVER ATTACK ADDS - Always prioritize healing
    if (botAI->IsHeal(bot))
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
        ObjectGuid botGuid = bot->GetGUID();
        ObjectGuid energyGuid = pureEnergy->GetGUID();
        uint32 currentTime = getMSTime();
        
        // EMERGENCY FALLBACK: Track if bot is stuck on same Pure Energy target
        if (g_pureEnergy_lastTarget[botGuid] == energyGuid)
        {
            // Same target - check if stuck
            if (g_pureEnergy_stuckTime[botGuid] == 0)
            {
                g_pureEnergy_stuckTime[botGuid] = currentTime;
            }
            else if ((currentTime - g_pureEnergy_stuckTime[botGuid]) > 5000U)
            {
                // Stuck for 5+ seconds - force target switch or fallback to boss
                g_pureEnergy_lastTarget[botGuid] = ObjectGuid::Empty;
                g_pureEnergy_stuckTime[botGuid] = 0;
                
                // Try to target boss instead
                Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
                if (boss)
                {
                    return Attack(boss);
                }
                return false;
            }
        }
        else
        {
            // New target - reset tracking
            g_pureEnergy_lastTarget[botGuid] = energyGuid;
            g_pureEnergy_stuckTime[botGuid] = 0;
        }
        
        // SPECIAL HANDLING: Pure Energy has REACT_PASSIVE
        // Use direct targeting instead of normal Attack() which might fail
        
        // Clear current target to reset combat state
        bot->SetTarget(pureEnergy->GetGUID());
        
        // Force threat generation for passive targets
        if (!pureEnergy->GetThreatMgr().GetThreat(bot))
        {
            pureEnergy->GetThreatMgr().AddThreat(bot, 1.0f);
        }
        
        // Use normal attack method
        return Attack(pureEnergy);
    }

    return false;
}

bool AttackPureEnergyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // CRITICAL: HEALERS SHOULD NEVER ATTACK ADDS - Always prioritize healing
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL FIX: Use direct trigger logic instead of Value system
    // The Value system may be failing to map trigger values properly
    
    // Check if Vexallus is in combat first
    Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // SMART TIMEOUT: Don't be useful if stuck on same target too long
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    // If we've been targeting Pure Energy for more than 10 seconds, something is wrong
    if (g_pureEnergy_lastSeenTime[botGuid] > 0 && 
        (currentTime - g_pureEnergy_lastSeenTime[botGuid]) > 10000U)
    {
        // Force timeout - let bots resume normal combat
        g_pureEnergy_lastSeenTime[botGuid] = 0;
        return false;
    }

    // ENHANCED DETECTION: Same logic as trigger to ensure consistency
    bool pureEnergyFound = false;

    // Method 1: Check hostile NPCs list for Pure Energy (most reliable method)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY)
        {
            pureEnergyFound = true;
            // Update last seen time for timeout system
            g_pureEnergy_lastSeenTime[botGuid] = currentTime;
            break;
        }
    }

    // Method 2: Direct creature search if not found in hostile list
    if (!pureEnergyFound)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 80.0f);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 80.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_PURE_ENERGY)
            {
                pureEnergyFound = true;
                // Update last seen time for timeout system
                g_pureEnergy_lastSeenTime[botGuid] = currentTime;
                break;
            }
        }
    }

    // CRITICAL FIX: DO NOT use aura detection - causes DPS freezing
    // Auras persist after creatures die, creating false positives that block normal combat
    // Only use reliable creature detection methods
    
    return pureEnergyFound;
}

// Vexallus spread
bool VexallusSpreadOutAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Tanks maintain position; avoid spreading for tanks
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* closest = nullptr;
    float closestDist = 1000.0f;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;
        float d = bot->GetExactDist2d(member);
        if (d < closestDist)
        {
            closest = member;
            closestDist = d;
        }
    }

    if (!closest || closestDist >= 8.0f)
        return false;

    // Step away from the closest member by ~6-8 yards
    float angle = bot->GetAngle(closest) + PI_F;
    float step = 6.0f + frand(0.0f, 2.0f);
    float x = bot->GetPositionX() + cos(angle) * step;
    float y = bot->GetPositionY() + sin(angle) * step;
    float z = bot->GetPositionZ();
    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool VexallusSpreadOutAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Healers and DPS should spread; tanks skip
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // If any teammate is closer than 8y, we should spread
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;
        if (bot->GetExactDist2d(member) < 8.0f)
            return true;
    }
    return false;
}

// Selin Fireheart Actions
bool AvoidFelExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
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

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    static const uint32 TARGET_LOCK_DURATION = 4000U;

    auto clearLock = [&]()
    {
        g_selin_crystalLock[botGuid] = ObjectGuid::Empty;
        g_selin_crystalLockTime[botGuid] = 0;
    };

    Unit* crystal = nullptr;

    if (g_selin_crystalLock[botGuid] && g_selin_crystalLockTime[botGuid] &&
        (currentTime - g_selin_crystalLockTime[botGuid]) < TARGET_LOCK_DURATION)
    {
        ObjectGuid lockedGuid = g_selin_crystalLock[botGuid];
        crystal = botAI->GetUnit(lockedGuid);
        if (!crystal)
            crystal = ObjectAccessor::GetUnit(*bot, lockedGuid);

        if (!crystal || !crystal->IsAlive() || crystal->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            clearLock();
            crystal = nullptr;
        }
    }

    if (!crystal)
        crystal = MagistersTerraceHelpers::SelectActiveFelCrystal(bot, botAI, boss);

    if (crystal)
    {
        g_selin_crystalLock[botGuid] = crystal->GetGUID();
        g_selin_crystalLockTime[botGuid] = currentTime;

        if (AiObjectContext* context = botAI->GetAiObjectContext())
        {
            if (auto* prioritized = context->GetValue<GuidVector>("prioritized targets"))
            {
                GuidVector focus;
                focus.push_back(crystal->GetGUID());
                prioritized->Set(focus);
            }
        }

        return Attack(crystal);
    }

    clearLock();
    return false;
}

bool AttackFelCrystalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    if (botAI->IsHeal(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 120.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (MagistersTerraceHelpers::SelectActiveFelCrystal(bot, botAI, boss))
        return true;

    if (boss->HasAura(SPELL_MANA_RAGE) ||
        (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE)))
        return true;

    return false;
}
// Delrissa Actions
bool AttackDelrissaAddAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 120.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        // Clear target lock when encounter is not active
        ObjectGuid botGuid = bot->GetGUID();
        g_delrissa_lockedTarget[botGuid] = ObjectGuid::Empty;
        g_delrissa_lockTime[botGuid] = 0;
        return false;
    }

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // TARGET LOCKING MECHANISM: Prevent rapid target switching
    static const uint32 TARGET_LOCK_DURATION = 5000U; // 5 second minimum lock

    // Check if we have a locked target that's still valid
    Unit* lockedTarget = nullptr;
    if (g_delrissa_lockedTarget[botGuid] && g_delrissa_lockTime[botGuid])
    {
        // Check if lock is still active
        if ((currentTime - g_delrissa_lockTime[botGuid]) < TARGET_LOCK_DURATION)
        {
            lockedTarget = botAI->GetUnit(g_delrissa_lockedTarget[botGuid]);

            // Validate locked target is still attackable
            if (lockedTarget && lockedTarget->IsAlive() &&
                AttackersValue::IsValidTarget(lockedTarget, bot) && bot->GetDistance(lockedTarget) < 50.0f)
            {
                // Continue attacking locked target
                return Attack(lockedTarget);
            }
            else
            {
                // Locked target is no longer valid, clear lock
                g_delrissa_lockedTarget[botGuid] = ObjectGuid::Empty;
                g_delrissa_lockTime[botGuid] = 0;
            }
        }
        else
        {
            // Lock expired, clear it
            g_delrissa_lockedTarget[botGuid] = ObjectGuid::Empty;
            g_delrissa_lockTime[botGuid] = 0;
        }
    }

    GuidVector hostileNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityTarget = SelectDelrissaHelperTarget(bot, botAI, hostileNpcs, true);

    if (!priorityTarget)
    {
        GuidVector possibleTargets = AI_VALUE(GuidVector, "possible targets");
        priorityTarget = SelectDelrissaHelperTarget(bot, botAI, possibleTargets, false);
    }

    // If we found a priority target, lock onto it
    if (priorityTarget)
    {
        // Lock the target to prevent switching
        g_delrissa_lockedTarget[botGuid] = priorityTarget->GetGUID();
        g_delrissa_lockTime[botGuid] = currentTime;

        return Attack(priorityTarget);
    }

    return false;
}

bool AttackDelrissaAddAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // CRITICAL: HEALERS SHOULD NEVER ATTACK ADDS - Always prioritize healing
    if (botAI->IsHeal(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 120.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    GuidVector hostileNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    if (SelectDelrissaHelperTarget(bot, botAI, hostileNpcs, false))
        return true;

    GuidVector possibleTargets = AI_VALUE(GuidVector, "possible targets");
    return SelectDelrissaHelperTarget(bot, botAI, possibleTargets, false) != nullptr;
}
// Interrupt dangerous helper abilities
bool InterruptDelrissaHelperAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 120.0f);
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
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 120.0f);
    return boss && boss->IsAlive() && boss->IsInCombat();
}


