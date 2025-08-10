#include "MagtheridonActions.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"

bool HellfireChannelerTargetAction::Execute(Event event)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Find all channelers
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* bestChanneler = nullptr;
    float lowestHealth = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            float healthPct = unit->GetHealthPct();
            if (healthPct < lowestHealth)
            {
                lowestHealth = healthPct;
                bestChanneler = unit;
            }
        }
    }
    
    if (bestChanneler && bestChanneler != currentTarget)
    {
        return botAI->Attack(bestChanneler);
    }
    
    return false;
}

bool HellfireChannelerTargetAction::isUseful()
{
    // Only target channelers during phase 1
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;
        
    return true;
}

bool InterruptDarkMendingAction::Execute(Event event)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_DARK_MENDING))
            {
                if (botAI->InterruptSpell(unit))
                    return true;
            }
        }
    }
    
    return false;
}

bool InterruptDarkMendingAction::isUseful()
{
    return bot->IsWithinMeleeRange(AI_VALUE(Unit*, "current target")) ||
           botAI->HasSpell(2139) || // Counterspell
           botAI->HasSpell(19647) || // Spell Lock
           botAI->HasSpell(1766); // Kick
}

bool ClickManticronCubeAction::Execute(Event event)
{
    if (!CanClickCube())
        return false;
        
    GameObject* cube = FindNearestCube();
    if (!cube)
        return false;
        
    // Move to cube if not in range
    if (bot->GetDistance(cube) > INTERACTION_DISTANCE)
    {
        return MoveTo(cube->GetMapId(), cube->GetPositionX(), cube->GetPositionY(), 
                     cube->GetPositionZ(), false, false, false, true, 
                     MovementPriority::MOVEMENT_COMBAT);
    }
    
    // Click the cube
    WorldPacket packet(CMSG_GAMEOBJ_USE);
    packet << cube->GetGUID();
    bot->GetSession()->HandleGameObjectUseOpcode(packet);
    
    botAI->SetActionDuration(12000); // Channel duration
    return true;
}

bool ClickManticronCubeAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Check if Blast Nova is casting
    if (!magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
        return false;
        
    return CanClickCube();
}

bool ClickManticronCubeAction::CanClickCube()
{
    // Check for exhaustion
    if (bot->HasAura(SPELL_MIND_EXHAUSTION))
        return false;
        
    // Check if already channeling
    if (bot->HasAura(SPELL_SHADOW_GRASP))
        return false;
        
    // Check if we're assigned to cube duty
    uint32 subGroup = bot->GetSubGroup();
    
    // Assign specific groups to cube duty (groups 1-5)
    if (subGroup >= 5)
        return false;
        
    return true;
}

GameObject* ClickManticronCubeAction::FindNearestCube()
{
    std::list<GameObject*> cubes;
    bot->GetGameObjectListWithEntryInGrid(cubes, GO_MANTICRON_CUBE, 50.0f);
    
    GameObject* nearest = nullptr;
    float minDist = 9999.0f;
    
    for (GameObject* cube : cubes)
    {
        if (!cube->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE))
        {
            float dist = bot->GetDistance(cube);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = cube;
            }
        }
    }
    
    return nearest;
}

bool StopClickingCubeAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_SHADOW_GRASP))
    {
        bot->InterruptNonMeleeSpells(true);
        return true;
    }
    
    return false;
}

bool StopClickingCubeAction::isUseful()
{
    if (!bot->HasAura(SPELL_SHADOW_GRASP))
        return false;
        
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return true; // Stop if no boss
        
    // Stop if boss is not casting blast nova anymore
    if (!magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
        return true;
        
    // Stop if banished
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE_STUN))
        return true;
        
    return false;
}

bool AvoidQuakeAction::Execute(Event event)
{
    Position safePos = GetSafePosition();
    
    return MoveTo(bot->GetMapId(), safePos.GetPositionX(), safePos.GetPositionY(),
                 safePos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool AvoidQuakeAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Check if Quake is being cast
    if (!magtheridon->FindCurrentSpellBySpellId(SPELL_QUAKE))
        return false;
        
    // Don't move if already far enough
    if (bot->GetDistance(magtheridon) > 30.0f)
        return false;
        
    return true;
}

Position AvoidQuakeAction::GetSafePosition()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return bot->GetPosition();
        
    float angle = bot->GetAngle(magtheridon) + M_PI;
    float distance = 35.0f;
    
    Position pos = magtheridon->GetPosition();
    pos.RelocatePolarOffset(angle, distance);
    
    return pos;
}

bool AvoidDebrisAction::Execute(Event event)
{
    Position safeSpot = FindSafeSpot();
    
    return MoveTo(bot->GetMapId(), safeSpot.GetPositionX(), safeSpot.GetPositionY(),
                 safeSpot.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool AvoidDebrisAction::isUseful()
{
    // Check for debris visual indicators
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        if (unit->GetEntry() == NPC_TARGET_TRIGGER)
        {
            if (unit->HasAura(SPELL_DEBRIS_VISUAL))
            {
                if (bot->GetDistance(unit) < 8.0f)
                    return true;
            }
        }
    }
    
    return false;
}

bool AvoidDebrisAction::IsDebrisFalling(Position const& pos)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        if (unit->GetEntry() == NPC_TARGET_TRIGGER)
        {
            if (unit->HasAura(SPELL_DEBRIS_VISUAL))
            {
                if (unit->GetDistance(pos) < 10.0f)
                    return true;
            }
        }
    }
    
    return false;
}

Position AvoidDebrisAction::FindSafeSpot()
{
    // Find a position away from all debris
    float bestAngle = 0;
    float bestDist = 0;
    
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 4)
    {
        for (float dist = 5.0f; dist <= 15.0f; dist += 5.0f)
        {
            Position testPos = bot->GetPosition();
            testPos.RelocatePolarOffset(angle, dist);
            
            if (!IsDebrisFalling(testPos))
            {
                bestAngle = angle;
                bestDist = dist;
                break;
            }
        }
    }
    
    Position safePos = bot->GetPosition();
    if (bestDist > 0)
    {
        safePos.RelocatePolarOffset(bestAngle, bestDist);
    }
    
    return safePos;
}

bool HandleCaveInAction::Execute(Event event)
{
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    // Reset movement state if enough time has passed
    if (g_caveInSafePosition[botGuid])
    {
        if ((currentTime - g_caveInLastMoveTime[botGuid]) > 15000)
        {
            g_caveInSafePosition[botGuid] = false;
        }
        else
        {
            return false; // Already safe
        }
    }
    
    // Move to center of room
    float centerX = -18.70f;
    float centerY = 2.24f;
    float centerZ = -0.15f;
    
    g_caveInLastMoveTime[botGuid] = currentTime;
    g_caveInSafePosition[botGuid] = true;
    
    return MoveTo(bot->GetMapId(), centerX, centerY, centerZ, false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool HandleCaveInAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Check if in phase 2 (30% health)
    if (magtheridon->GetHealthPct() > 30.0f)
        return false;
        
    // Check for camera shake (precursor to cave in)
    if (bot->HasAura(SPELL_CAMERA_SHAKE))
        return true;
        
    return false;
}

bool AvoidBlazeAction::Execute(Event event)
{
    Position safePos = GetAwayFromBlaze();
    
    return MoveTo(bot->GetMapId(), safePos.GetPositionX(), safePos.GetPositionY(),
                 safePos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool AvoidBlazeAction::isUseful()
{
    return IsBlazeNearby();
}

bool AvoidBlazeAction::IsBlazeNearby()
{
    // Check for Blaze ground effect
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Blaze summon creates a trigger
        if (unit->HasAura(SPELL_BLAZE_SUMMON) || unit->GetEntry() == 17653) // Blaze trigger
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }
    
    return false;
}

Position AvoidBlazeAction::GetAwayFromBlaze()
{
    Position bestPos = bot->GetPosition();
    float maxDist = 0;
    
    // Find direction away from all blazes
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
    {
        Position testPos = bot->GetPosition();
        testPos.RelocatePolarOffset(angle, 10.0f);
        
        float minBlazeDist = 999.0f;
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit)
                continue;
                
            if (unit->HasAura(SPELL_BLAZE_SUMMON) || unit->GetEntry() == 17653)
            {
                float dist = testPos.GetDistance(unit->GetPosition());
                if (dist < minBlazeDist)
                    minBlazeDist = dist;
            }
        }
        
        if (minBlazeDist > maxDist)
        {
            maxDist = minBlazeDist;
            bestPos = testPos;
        }
    }
    
    return bestPos;
}

bool SpreadForCleaveAction::Execute(Event event)
{
    Position spreadPos = GetSpreadPosition();
    
    return MoveTo(bot->GetMapId(), spreadPos.GetPositionX(), spreadPos.GetPositionY(),
                 spreadPos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool SpreadForCleaveAction::isUseful()
{
    if (botAI->IsTank(bot))
        return false;
        
    return IsTooCloseToTank();
}

bool SpreadForCleaveAction::IsTooCloseToTank()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
        
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot)
            continue;
            
        if (botAI->IsTank(member))
        {
            if (bot->GetDistance(member) < 8.0f && bot->IsWithinMeleeRange(magtheridon))
                return true;
        }
    }
    
    return false;
}

Position SpreadForCleaveAction::GetSpreadPosition()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return bot->GetPosition();
        
    // Move to the side of the boss
    float angle = magtheridon->GetAngle(bot);
    angle += (bot->GetGUID().GetCounter() % 2 == 0) ? M_PI / 2 : -M_PI / 2;
    
    Position pos = magtheridon->GetPosition();
    pos.RelocatePolarOffset(angle, 10.0f);
    
    return pos;
}

bool HellfireWarderTargetAction::Execute(Event event)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_WARDER)
        {
            return botAI->Attack(unit);
        }
    }
    
    return false;
}

bool HellfireWarderTargetAction::isUseful()
{
    // Warders appear when channelers are engaged
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_WARDER)
            return true;
    }
    
    return false;
}

bool TankPositionAddsAction::Execute(Event event)
{
    Position tankPos = GetTankPosition();
    
    return MoveTo(bot->GetMapId(), tankPos.GetPositionX(), tankPos.GetPositionY(),
                 tankPos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool TankPositionAddsAction::isUseful()
{
    if (!botAI->IsTank(bot))
        return false;
        
    // Check if we have adds to tank
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return false;
        
    uint32 entry = currentTarget->GetEntry();
    if (entry != NPC_HELLFIRE_CHANNELER && entry != NPC_HELLFIRE_WARDER)
        return false;
        
    return true;
}

Position TankPositionAddsAction::GetTankPosition()
{
    // Tank adds away from the raid
    float tankX = -35.0f;
    float tankY = 15.0f;
    float tankZ = -0.15f;
    
    return Position(tankX, tankY, tankZ, 0);
}

bool HealerPositionMagtheridonAction::Execute(Event event)
{
    Position healPos = GetHealerSafeSpot();
    
    return MoveTo(bot->GetMapId(), healPos.GetPositionX(), healPos.GetPositionY(),
                 healPos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool HealerPositionMagtheridonAction::isUseful()
{
    if (!botAI->IsHealer(bot))
        return false;
        
    // Check if too close to danger
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && bot->GetDistance(magtheridon) < 20.0f)
        return true;
        
    return false;
}

Position HealerPositionMagtheridonAction::GetHealerSafeSpot()
{
    // Healers spread around the room at max range
    float angle = (bot->GetGUID().GetCounter() % 8) * (M_PI / 4);
    float distance = 35.0f;
    
    Position pos(-18.70f, 2.24f, -0.15f, 0);
    pos.RelocatePolarOffset(angle, distance);
    
    return pos;
}

bool BanishPhasePositionAction::Execute(Event event)
{
    Position banishPos = GetBanishPosition();
    
    return MoveTo(bot->GetMapId(), banishPos.GetPositionX(), banishPos.GetPositionY(),
                 banishPos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool BanishPhasePositionAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // During banish, spread for safety
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE_STUN))
        return true;
        
    return false;
}

Position BanishPhasePositionAction::GetBanishPosition()
{
    // Spread evenly during banish
    uint32 index = bot->GetGUID().GetCounter() % 16;
    float angle = index * (2 * M_PI / 16);
    float distance = 25.0f;
    
    Position pos(-18.70f, 2.24f, -0.15f, 0);
    pos.RelocatePolarOffset(angle, distance);
    
    return pos;
}

bool AvoidInfernalAction::Execute(Event event)
{
    Position safePos = GetSafeFromInfernal();
    
    return MoveTo(bot->GetMapId(), safePos.GetPositionX(), safePos.GetPositionY(),
                 safePos.GetPositionZ(), false, false, false, true,
                 MovementPriority::MOVEMENT_COMBAT);
}

bool AvoidInfernalAction::isUseful()
{
    return IsInfernalNearby();
}

bool AvoidInfernalAction::IsInfernalNearby()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        // Check for Burning Abyssal
        if (unit->GetEntry() == 17454) // Burning Abyssal entry
        {
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }
    
    return false;
}

Position AvoidInfernalAction::GetSafeFromInfernal()
{
    Position bestPos = bot->GetPosition();
    float maxDist = 0;
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
    {
        Position testPos = bot->GetPosition();
        testPos.RelocatePolarOffset(angle, 15.0f);
        
        float minInfernalDist = 999.0f;
        
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == 17454)
            {
                float dist = testPos.GetDistance(unit->GetPosition());
                if (dist < minInfernalDist)
                    minInfernalDist = dist;
            }
        }
        
        if (minInfernalDist > maxDist)
        {
            maxDist = minInfernalDist;
            bestPos = testPos;
        }
    }
    
    return bestPos;
}

bool CoordinateChannelerInterruptAction::Execute(Event event)
{
    Unit* channeler = FindChannelerCasting();
    if (!channeler)
        return false;
        
    if (ShouldIInterrupt(channeler))
    {
        return botAI->InterruptSpell(channeler);
    }
    
    return false;
}

bool CoordinateChannelerInterruptAction::isUseful()
{
    return FindChannelerCasting() != nullptr;
}

Unit* CoordinateChannelerInterruptAction::FindChannelerCasting()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_DARK_MENDING) ||
                unit->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY))
            {
                return unit;
            }
        }
    }
    
    return nullptr;
}

bool CoordinateChannelerInterruptAction::ShouldIInterrupt(Unit* channeler)
{
    // Check if other bots are already interrupting
    Group* group = bot->GetGroup();
    if (!group)
        return true; // Solo, always interrupt
        
    uint32 interruptersInRange = 0;
    uint32 myIndex = 999;
    uint32 currentIndex = 0;
    
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
            
        if (member->GetDistance(channeler) > 30.0f)
            continue;
            
        // Check if they can interrupt
        if (member->getClass() == CLASS_ROGUE ||
            member->getClass() == CLASS_WARRIOR ||
            member->getClass() == CLASS_MAGE ||
            member->getClass() == CLASS_WARLOCK ||
            member->getClass() == CLASS_SHAMAN)
        {
            if (member == bot)
                myIndex = currentIndex;
                
            interruptersInRange++;
            currentIndex++;
        }
    }
    
    // Rotate interrupts among capable players
    if (myIndex == 0 || interruptersInRange == 1)
        return true;
        
    return false;
}

bool MagtheridonPhaseTransitionAction::Execute(Event event)
{
    uint8 newPhase = GetCurrentPhase();
    HandlePhaseChange(newPhase);
    
    return true;
}

bool MagtheridonPhaseTransitionAction::isUseful()
{
    static uint8 lastPhase = 0;
    uint8 currentPhase = GetCurrentPhase();
    
    if (currentPhase != lastPhase)
    {
        lastPhase = currentPhase;
        return true;
    }
    
    return false;
}

uint8 MagtheridonPhaseTransitionAction::GetCurrentPhase()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 0;
        
    // Phase 0: Channelers alive, Magtheridon caged
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return 0;
        
    // Phase 2: Below 30% health
    if (magtheridon->GetHealthPct() <= 30.0f)
        return 2;
        
    // Phase 1: Released and fighting
    return 1;
}

void MagtheridonPhaseTransitionAction::HandlePhaseChange(uint8 newPhase)
{
    switch (newPhase)
    {
        case 0:
            // Focus channelers
            botAI->ChangeStrategy("+magtheridon channelers", BotState::BOT_STATE_COMBAT);
            break;
        case 1:
            // Magtheridon released
            botAI->ChangeStrategy("-magtheridon channelers", BotState::BOT_STATE_COMBAT);
            botAI->ChangeStrategy("+magtheridon released", BotState::BOT_STATE_COMBAT);
            break;
        case 2:
            // Cave in phase
            botAI->ChangeStrategy("+magtheridon cave in", BotState::BOT_STATE_COMBAT);
            break;
    }
}