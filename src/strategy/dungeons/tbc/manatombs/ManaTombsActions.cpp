#include "ManaTombsActions.h"
#include "Playerbots.h"

// Pandemonius - Dark Shell reflects damage
bool PandemoniusDarkShellAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Dark Shell reflects damage every 20s - boss_pandemonius.cpp:70-80
    // Check if casting or has aura
    if (boss->FindCurrentSpellBySpellId(SPELL_DARK_SHELL) || boss->HasAura(SPELL_DARK_SHELL))
    {
        // Stop ALL attacks immediately to avoid reflection
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        
        // Clear target to prevent auto-attacks
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        
        // Move away and wait
        if (bot->GetDistance(boss) < 30.0f)
        {
            float angle = boss->GetAngle(bot) + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 35.0f;
            float y = boss->GetPositionY() + sin(angle) * 35.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool PandemoniusDarkShellAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasAura(SPELL_DARK_SHELL);
}

// Spread for Void Blast
bool PandemoniusVoidBlastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Void Blast fires 5 times rapidly - boss_pandemonius.cpp:81-93
    // Must maintain good spread throughout fight
    if (boss->FindCurrentSpellBySpellId(SPELL_VOID_BLAST))
    {
        // During Void Blast barrage, spread out more
        float minSafeDistance = 12.0f;
        
        Group* group = bot->GetGroup();
        if (group)
        {
            // Find optimal position away from all allies
            float bestAngle = 0;
            float maxMinDist = 0;
            
            for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
            {
                float testX = boss->GetPositionX() + cos(angle) * 20.0f;
                float testY = boss->GetPositionY() + sin(angle) * 20.0f;
                float minDist = 100.0f;
                
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
                
                if (minDist > maxMinDist)
                {
                    maxMinDist = minDist;
                    bestAngle = angle;
                }
            }
            
            if (maxMinDist < minSafeDistance)
            {
                float destX = boss->GetPositionX() + cos(bestAngle) * 20.0f;
                float destY = boss->GetPositionY() + sin(bestAngle) * 20.0f;
                return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                            false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool PandemoniusVoidBlastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    return boss && boss->IsAlive() && boss->IsInCombat();
}

// Tavarok - Earthquake AoE
bool TavarokEarthquakeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Earthquake has 2 sec cast - boss_tavarok.cpp:48-50
    // Move preemptively when cast starts
    if (boss->FindCurrentSpellBySpellId(SPELL_EARTHQUAKE))
    {
        // Earthquake affects large area around boss
        float safeDistance = 25.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            // Move away quickly
            float angle = boss->GetAngle(bot) + M_PI;
            float moveDistance = safeDistance - currentDist + 5.0f;
            float x = boss->GetPositionX() + cos(angle) * (currentDist + moveDistance);
            float y = boss->GetPositionY() + sin(angle) * (currentDist + moveDistance);
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool TavarokEarthquakeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->FindCurrentSpellBySpellId(SPELL_EARTHQUAKE) && bot->GetExactDist2d(boss) < 25.0f;
}

// Break Crystal Prison on allies
bool TavarokCrystalPrisonAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Crystal Prison needs to be DPSed to break - boss_tavarok.cpp:53
    // Look for Crystal Prison gameobjects, not player auras
    std::list<GameObject*> prisonList;
    bot->GetGameObjectListWithEntryInGrid(prisonList, 181278, 50.0f); // Crystal Prison object
    
    for (GameObject* prison : prisonList)
    {
        if (prison && prison->GetGoState() == GO_STATE_READY)
        {
            // Move to prison and attack it
            if (bot->GetDistance(prison) > 5.0f)
            {
                return MoveTo(bot->GetMapId(), prison->GetPositionX(), prison->GetPositionY(), 
                            prison->GetPositionZ(), false, false, false, true, 
                            MovementPriority::MOVEMENT_FORCED);
            }
            else
            {
                // Focus DPS on the prison
                bot->SetFacingToObject(prison);
                // Note: Actual attacking of gameobject handled by client
                return true;
            }
        }
    }

    return false;
}

bool TavarokCrystalPrisonAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member != bot && member->IsAlive() && member->HasAura(SPELL_CRYSTAL_PRISON))
            return true;
    }

    return false;
}

// Avoid Arcing Smash frontal cleave
bool TavarokArcingSmashAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Arcing Smash every 8-12s - boss_tavarok.cpp:56-59
    // Check if casting OR if we're in danger zone
    if (boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH) || 
        (botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot)))
    {
        // Non-tanks should never be in front
        if (!botAI->IsTank(bot) && boss->HasInArc(M_PI / 2, bot))
        {
            // Move behind boss immediately
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool TavarokArcingSmashAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return (boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH) || 
            (!botAI->IsTank(bot) && botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot)));
}

// Nexus-Prince Shaffar - Attack Ethereal Beacons
bool AttackEtherealBeaconAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Beacons spawn every 10s - boss_nexusprince_shaffar.cpp:86-93
    // Priority target - they spawn adds if not killed
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Find all beacons and target the oldest one
    std::list<Creature*> beaconList;
    bot->GetCreatureListWithEntryInGrid(beaconList, NPC_ETHEREAL_BEACON, 50.0f);
    
    Unit* oldestBeacon = nullptr;
    uint32 oldestTime = 0;
    
    for (Creature* beacon : beaconList)
    {
        if (beacon && beacon->IsAlive())
        {
            // Target beacon immediately if not already
            if (currentTarget != beacon)
            {
                return Attack(beacon);
            }
        }
    }

    return false;
}

bool AttackEtherealBeaconAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* beacon = bot->FindNearestCreature(NPC_ETHEREAL_BEACON, 50.0f);
    return beacon && beacon->IsAlive();
}

// Avoid Frost Nova
bool ShaffarFrostNovaAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Frost Nova followed by Blink - boss_nexusprince_shaffar.cpp:98-106
    // Move out BEFORE cast completes
    if (boss->FindCurrentSpellBySpellId(SPELL_FROSTNOVA))
    {
        float distance = bot->GetExactDist2d(boss);
        if (distance < 20.0f) // Frost Nova has ~10 yard range, be safe
        {
            // Move away immediately - boss will blink after nova
            float angle = boss->GetAngle(bot) + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 25.0f;
            float y = boss->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool ShaffarFrostNovaAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->FindCurrentSpellBySpellId(SPELL_FROSTNOVA) && bot->GetExactDist2d(boss) < 20.0f;
}

// Reposition after Shaffar blinks
bool ShaffarBlinkAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Blinks away after Frost Nova - boss_nexusprince_shaffar.cpp:103-106
    // Check if boss blinked away (sudden distance change)
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    if (currentTarget == boss)
    {
        float distance = bot->GetExactDist2d(boss);
        // If suddenly out of range, re-engage
        if (distance > 30.0f || (botAI->IsMelee(bot) && distance > 10.0f))
        {
            // Move to boss new position
            return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(),
                        boss->GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool ShaffarBlinkAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if we're out of range after blink
    return bot->GetDistance(boss) > 30.0f;
}

// Yor - Avoid Double Breath frontal cone
bool YorDoubleBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Double Breath only when target in melee - boss_nexusprince_shaffar.cpp:166-169
    // Non-tanks should stay behind always
    if (!botAI->IsTank(bot) && botAI->IsMelee(bot))
    {
        // Always position behind Yor
        if (boss->HasInArc(M_PI / 2, bot))
        {
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // If Double Breath is casting, ranged should also avoid front
    if (boss->FindCurrentSpellBySpellId(SPELL_DOUBLE_BREATH))
    {
        if (boss->HasInArc(M_PI / 3, bot))
        {
            float angle = boss->GetOrientation() + (frand(0, 1) > 0.5f ? M_PI/2 : -M_PI/2);
            float x = boss->GetPositionX() + cos(angle) * 15.0f;
            float y = boss->GetPositionY() + sin(angle) * 15.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool YorDoubleBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return (boss->FindCurrentSpellBySpellId(SPELL_DOUBLE_BREATH) && boss->HasInArc(M_PI / 3, bot)) ||
           (!botAI->IsTank(bot) && botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot));
}

// Avoid Stomp AoE
bool YorStompAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Stomp every 14-24s - boss_nexusprince_shaffar.cpp:171-174
    // Has visible cast time, move out preemptively
    if (boss->FindCurrentSpellBySpellId(SPELL_STOMP))
    {
        float distance = bot->GetExactDist2d(boss);
        float safeDistance = 20.0f; // Stomp has ~15 yard range
        
        if (distance < safeDistance)
        {
            // Move away quickly
            float angle = boss->GetAngle(bot) + M_PI;
            float moveDistance = safeDistance - distance + 5.0f;
            float x = boss->GetPositionX() + cos(angle) * (distance + moveDistance);
            float y = boss->GetPositionY() + sin(angle) * (distance + moveDistance);
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool YorStompAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->FindCurrentSpellBySpellId(SPELL_STOMP) && bot->GetExactDist2d(boss) < 20.0f;
}