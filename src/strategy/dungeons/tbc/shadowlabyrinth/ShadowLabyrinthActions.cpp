#include "Playerbots.h"
#include "ShadowLabyrinthActions.h"
#include "ShadowLabyrinthStrategy.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

// Per-bot state maps for Void Traveler timeout mechanism
std::map<ObjectGuid, uint32> g_voidTraveler_lastSeenTime;
std::map<ObjectGuid, uint32> g_voidTraveler_stuckTime;
std::map<ObjectGuid, ObjectGuid> g_voidTraveler_lastTarget;


bool AvoidCorrosiveAcidAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss)
    {
        return false;
    }
    
    // Corrosive Acid is a frontal cone on random target, spread out
    if (boss->FindCurrentSpellBySpellId(SPELL_CORROSIVE_ACID))
    {
        // Check if other players are near us
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 8.0f)
                    {
                        // Move away from other players to avoid acid spread
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 10.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 10.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
        }
    }
    
    return false;
}

bool HellmawFearReactAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // Check if boss is casting Fear (33547) and spread preemptively
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_FEAR))
    {
        // Spread out to minimize fear chains
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 10.0f)
                    {
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 12.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 12.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
        }
    }
    
    return false;
}

bool InciteChaosReactAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // During Incite Chaos, players attack each other AND boss becomes immune - spread out!
    if (boss->FindCurrentSpellBySpellId(SPELL_INCITE_CHAOS) || bot->HasAura(SPELL_INCITE_CHAOS_B))
    {
        // CRITICAL: Boss is immune during Incite Chaos - stop attacking boss
        if (boss->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) || boss->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC))
        {
            // Clear target if currently attacking immune boss to prevent wasted abilities
            Unit* currentTarget = AI_VALUE(Unit*, "current target");
            if (currentTarget && currentTarget->GetGUID() == boss->GetGUID())
            {
                botAI->ChangeStrategy("-follow,+stay", BotState::BOT_STATE_COMBAT);
                // Stop attacking boss during immunity - will resume after chaos ends
            }
        }
        // Move away from all other players to minimize damage
        float bestAngle = 0;
        float maxMinDist = 0;
        
        for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
        {
            float testX = boss->GetPositionX() + cos(angle) * 20.0f;
            float testY = boss->GetPositionY() + sin(angle) * 20.0f;
            float minDist = 100.0f;
            
            Group* group = bot->GetGroup();
            if (group)
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member != bot && member->IsAlive())
                    {
                        float dist = member->GetExactDist2d(testX, testY);
                        if (dist < minDist)
                            minDist = dist;
                    }
                }
            }
            
            if (minDist > maxMinDist)
            {
                maxMinDist = minDist;
                bestAngle = angle;
            }
        }
        
        float destX = boss->GetPositionX() + cos(bestAngle) * 20.0f;
        float destY = boss->GetPositionY() + sin(bestAngle) * 20.0f;
        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool AvoidWarStompAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // REACTIVE WAR STOMP AVOIDANCE: Move away when boss actually casts War Stomp
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_WAR_STOMP))
    {
        float currentDistance = bot->GetExactDist2d(boss);
        if (currentDistance < 12.0f) // Within stomp range
        {
            // Move directly away from boss to avoid the stomp
            float angle = boss->GetAngle(bot) + M_PI; // Opposite direction
            float moveDistance = 15.0f - currentDistance; // Get outside 12+ yard range
            float destX = bot->GetPositionX() + cos(angle) * moveDistance;
            float destY = bot->GetPositionY() + sin(angle) * moveDistance;
            
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool BlackheartChargeReactAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // CHARGE REACTION: Boss charges at bot, need to prepare for impact and reposition
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_CHARGE))
    {
        // Check if this bot is being charged
        if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (currentSpell && currentSpell->GetSpellInfo()->Id == SL_SPELL_CHARGE)
            {
                Unit* target = currentSpell->m_targets.GetUnitTarget();
                if (target && target->GetGUID() == bot->GetGUID())
                {
                    // Bot is being charged - notify group by moving slightly to indicate threat
                    // This helps tanks regain aggro after charge
                    
                    // Move perpendicular to boss to avoid being in direct line after charge
                    float angle = boss->GetAngle(bot) + (M_PI / 2.0f); // 90 degrees
                    float destX = bot->GetPositionX() + cos(angle) * 8.0f;
                    float destY = bot->GetPositionY() + sin(angle) * 8.0f;
                    
                    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }
    
    return false;
}

bool VoidTravelerPriorityAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // CRITICAL: HEALERS SHOULD NEVER ATTACK ADDS - Always prioritize healing
    if (botAI->IsHeal(bot))
        return false;

    // Check if Grandmaster Vorpil encounter is active
    Unit* boss = bot->FindNearestCreature(NPC_GRANDMASTER_VORPIL, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // SMART TIMEOUT: Don't be useful if stuck on same target too long
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    // If we've been targeting Void Travelers for more than 20 seconds, something is wrong
    if (g_voidTraveler_lastSeenTime[botGuid] > 0 && 
        (currentTime - g_voidTraveler_lastSeenTime[botGuid]) > 20000U)
    {
        // Force timeout - let bots resume normal combat
        g_voidTraveler_lastSeenTime[botGuid] = 0;
        return false;
    }

    // ENHANCED DETECTION: Same logic as trigger to ensure consistency
    bool voidTravelerFound = false;

    // Method 1: Check hostile NPCs list for Void Travelers (most reliable method)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_VOID_TRAVELER)
        {
            voidTravelerFound = true;
            // Update last seen time for timeout system
            g_voidTraveler_lastSeenTime[botGuid] = currentTime;
            break;
        }
    }

    // Method 2: Direct creature search if not found in hostile list
    if (!voidTravelerFound)
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

            if (unit->GetEntry() == NPC_VOID_TRAVELER)
            {
                voidTravelerFound = true;
                // Update last seen time for timeout system
                g_voidTraveler_lastSeenTime[botGuid] = currentTime;
                break;
            }
        }
    }
    
    return voidTravelerFound;
}

bool VoidTravelerPriorityAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // CRITICAL: HEALERS NEVER ATTACK ADDS - Always prioritize healing
    if (botAI->IsHeal(bot))
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    // ENHANCED VOID TRAVELER TARGETING: Multi-method approach for REACT_PASSIVE
    Unit* voidTraveler = nullptr;
    float closestDistToBoss = 100.0f;

    // Method 1: Check hostile NPCs list first (most reliable)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_VOID_TRAVELER)
        {
            float distToBoss = unit->GetExactDist2d(boss);
            if (distToBoss < closestDistToBoss)
            {
                closestDistToBoss = distToBoss;
                voidTraveler = unit;
            }
        }
    }

    // Method 2: Direct creature search if no void traveler found in hostile list
    if (!voidTraveler)
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

            if (unit->GetEntry() == NPC_VOID_TRAVELER)
            {
                float distToBoss = unit->GetExactDist2d(boss);
                if (distToBoss < closestDistToBoss)
                {
                    closestDistToBoss = distToBoss;
                    voidTraveler = unit;
                }
            }
        }
    }

    // Method 3: Emergency fallback - find ANY Void Traveler if none found above
    if (!voidTraveler)
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

            // Accept ANY Void Traveler as emergency target
            if (unit->GetEntry() == NPC_VOID_TRAVELER)
            {
                float distToBoss = unit->GetExactDist2d(boss);
                if (distToBoss < closestDistToBoss)
                {
                    closestDistToBoss = distToBoss;
                    voidTraveler = unit;
                }
            }
        }
    }

    if (voidTraveler)
    {
        ObjectGuid botGuid = bot->GetGUID();
        ObjectGuid travelerGuid = voidTraveler->GetGUID();
        uint32 currentTime = getMSTime();
        
        // EMERGENCY FALLBACK: Track if bot is stuck on same Void Traveler target
        if (g_voidTraveler_lastTarget[botGuid] == travelerGuid)
        {
            // Same target - check if stuck
            if (g_voidTraveler_stuckTime[botGuid] == 0)
            {
                g_voidTraveler_stuckTime[botGuid] = currentTime;
            }
            else if ((currentTime - g_voidTraveler_stuckTime[botGuid]) > 15000U)
            {
                // Stuck for 15+ seconds - force target switch or fallback to boss
                g_voidTraveler_lastTarget[botGuid] = ObjectGuid::Empty;
                g_voidTraveler_stuckTime[botGuid] = 0;
                
                return false;
            }
        }
        else
        {
            // New target - reset tracking
            g_voidTraveler_lastTarget[botGuid] = travelerGuid;
            g_voidTraveler_stuckTime[botGuid] = 0;
        }
        
        // SPECIAL HANDLING: Void Travelers have REACT_PASSIVE
        // Use direct targeting instead of normal Attack() which might fail
        
        // Clear current target to reset combat state
        bot->SetTarget(voidTraveler->GetGUID());
        
        // Force threat generation for passive targets
        if (!voidTraveler->GetThreatMgr().GetThreat(bot))
        {
            voidTraveler->GetThreatMgr().AddThreat(bot, 1.0f);
        }
        
        // Use normal attack method
        return Attack(voidTraveler);
    }
    
    return false;
}

bool MoveFromRainOfFireAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss)
    {
        return false;
    }
    
    // Rain of Fire is cast at center after teleport
    if (boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE))
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float safeRadius = 15.0f; // Rain of Fire radius plus safety margin
        float currentDist = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
        
        if (currentDist < safeRadius)
        {
            // Move away from center immediately
            float angle = atan2(bot->GetPositionY() - centerPos.GetPositionY(),
                              bot->GetPositionX() - centerPos.GetPositionX());
            float destX = centerPos.GetPositionX() + cos(angle) * (safeRadius + 3.0f);
            float destY = centerPos.GetPositionY() + sin(angle) * (safeRadius + 3.0f);
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // Also check if we're standing in Rain of Fire (ground effect)
    if (bot->HasAura(33617)) // Rain of Fire damage aura
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float angle = atan2(bot->GetPositionY() - centerPos.GetPositionY(),
                          bot->GetPositionX() - centerPos.GetPositionX());
        float destX = centerPos.GetPositionX() + cos(angle) * 18.0f;
        float destY = centerPos.GetPositionY() + sin(angle) * 18.0f;
        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool DrawShadowsReactAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss)
    {
        return false;
    }
    
    // Draw Shadows teleports boss to center, then casts Rain of Fire
    // Move away from center immediately when Draw Shadows is cast
    if (boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float safeDistance = 15.0f; // Rain of Fire has ~12 yard radius
        float currentDist = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
        
        if (currentDist < safeDistance)
        {
            // Move directly away from center
            float angle = atan2(bot->GetPositionY() - centerPos.GetPositionY(), 
                              bot->GetPositionX() - centerPos.GetPositionX());
            float destX = centerPos.GetPositionX() + cos(angle) * (safeDistance + 3.0f);
            float destY = centerPos.GetPositionY() + sin(angle) * (safeDistance + 3.0f);
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool MurmurSonicBoomAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    // CRITICAL: Sonic Boom does 80% of CURRENT health damage!
    // Must be 34+ yards away to avoid it
    if (boss->FindCurrentSpellBySpellId(SPELL_SONIC_BOOM_CAST))
    {
        float safeDistance = 36.0f; // 34 yards + safety margin
        float currentDist = bot->GetExactDist(boss);
        
        if (currentDist < safeDistance)
        {
            // Move directly away from boss FAST
            float angle = boss->GetAngle(bot) + M_PI;
            float moveDistance = safeDistance - currentDist + 2.0f;
            float destX = boss->GetPositionX() + cos(angle) * (currentDist + moveDistance);
            float destY = boss->GetPositionY() + sin(angle) * (currentDist + moveDistance);
            float destZ = boss->GetPositionZ();
            
            // Force immediate movement with highest priority
            return MoveTo(boss->GetMapId(), destX, destY, destZ, 
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool MurmurResonanceAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    // Resonance is cast when no one is in melee range
    // Tank must stay close to prevent it
    if (botAI->IsTank(bot))
    {
        float meleeRange = 5.0f;
        float currentDist = bot->GetExactDist(boss);
        
        // Always stay in melee range as tank
        if (currentDist > meleeRange)
        {
            return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), 
                         boss->GetPositionZ(), false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (boss->FindCurrentSpellBySpellId(SPELL_RESONANCE))
    {
        // Non-tanks should spread to minimize damage
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 10.0f)
                    {
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 12.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 12.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
        }
    }
    
    return false;
}

bool MurmurMagneticPullAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    // Magnetic Pull brings a player to Murmur, followed by Murmur's Touch
    // The touched player needs to move away from others to avoid spread
    if (bot->HasAura(SPELL_MURMURS_TOUCH) || bot->HasAura(SL_SPELL_MAGNETIC_PULL))
    {
        // Move away from other players to avoid Murmur's Touch explosion
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 15.0f) // Murmur's Touch has splash damage
                    {
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 20.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 20.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
        }
    }
    
    return false;
}

bool MurmurThunderingStormAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    // Thundering Storm targets players 25-100 yards away (Heroic only)
    // Best position is either very close (<25y) or very far (>100y)
    if (boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM))
    {
        float currentDist = bot->GetExactDist(boss);
        
        // If in danger zone (25-100 yards), move closer or farther
        if (currentDist >= 25.0f && currentDist <= 100.0f)
        {
            // Tanks and melee move in close
            if (botAI->IsTank(bot) || botAI->IsMelee(bot))
            {
                // Move closer to boss
                return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), 
                                 boss->GetPositionZ(), false, false, false, true, 
                                 MovementPriority::MOVEMENT_FORCED);
            }
            // Ranged try to get farther if possible
            else
            {
                float angle = boss->GetAngle(bot) + M_PI;
                float destX = boss->GetPositionX() + cos(angle) * 110.0f;
                float destY = boss->GetPositionY() + sin(angle) * 110.0f;
                return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}