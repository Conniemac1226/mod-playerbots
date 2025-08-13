#include "BlackTempleActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Unit.h"
#include "GameObject.h"
#include "ObjectMgr.h"

// Illidan Stormrage
static std::map<ObjectGuid, uint8> g_illidan_currentPhase;
static std::map<ObjectGuid, time_t> g_illidan_lastFlameAvoid;
static std::map<ObjectGuid, bool> g_illidan_hasDrawSoul;
static std::map<ObjectGuid, time_t> g_illidan_lastShadowDemonTime;

bool IllidanPhaseCheckAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Detect phase based on Illidan's state
    uint8 phase = 1;
    
    // Phase 2: Flying (Eye Beam phase)
    if (illidan->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
    {
        phase = 2;
    }
    // Phase 4: Demon Form
    else if (illidan->HasAura(40506)) // SPELL_DEMON_FORM
    {
        phase = 4;
    }
    // Phase 5: Enrage (Frenzy)
    else if (illidan->HasAura(40683)) // SPELL_FRENZY
    {
        phase = 5;
    }
    // Phase 3: Maiev active
    else
    {
        Unit* maiev = AI_VALUE2(Unit*, "find target", "maiev shadowsong");
        if (maiev && maiev->IsAlive())
        {
            phase = 3;
        }
    }

    g_illidan_currentPhase[botGuid] = phase;
    return true;
}

bool IllidanFlameCrashAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if Illidan is casting Flame Crash
    const uint32 SPELL_FLAME_CRASH = 40832;
    if (!illidan->FindCurrentSpellBySpellId(SPELL_FLAME_CRASH))
        return false;

    // Avoid spam movement
    time_t now = time(nullptr);
    if ((now - g_illidan_lastFlameAvoid[botGuid]) < 3)
        return false;

    // Move away from Illidan's position (frontal cone)
    float distance = bot->GetDistance(illidan);
    if (distance < 20.0f)
    {
        float angle = illidan->GetAngle(bot);
        float moveAngle = angle + M_PI; // Move directly away
        
        Position movePos = illidan->GetFirstCollisionPosition(25.0f, moveAngle);
        bot->GetMotionMaster()->MovePoint(0, movePos);
        
        g_illidan_lastFlameAvoid[botGuid] = now;
        return true;
    }

    return false;
}

bool IllidanParasiticShadowfiendAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Find and kill Parasitic Shadowfiends quickly
    const uint32 NPC_PARASITIC_SHADOWFIEND = 23498;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
    
    GuidVector npcs = npcsValue->Get();
    
    Unit* closestShadowfiend = nullptr;
    float closestDistance = 40.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PARASITIC_SHADOWFIEND)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestShadowfiend = unit;
            }
        }
    }

    if (closestShadowfiend)
    {
        if (AI_VALUE(Unit*, "current target") != closestShadowfiend)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(closestShadowfiend);
            bot->SetTarget(closestShadowfiend->GetGUID());
        }
        return true;
    }

    return false;
}

bool IllidanDrawSoulHealAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if we have Draw Soul debuff
    const uint32 SPELL_DRAW_SOUL = 40904;
    if (!bot->HasAura(SPELL_DRAW_SOUL))
    {
        g_illidan_hasDrawSoul[botGuid] = false;
        return false;
    }

    // Focus healing on Draw Soul target
    if (!g_illidan_hasDrawSoul[botGuid])
    {
        g_illidan_hasDrawSoul[botGuid] = true;
        
        // Healers should prioritize this target
        if (botAI->IsHeal(bot))
        {
            return botAI->CastSpell("flash heal", bot);
        }
    }

    return false;
}

bool IllidanAgonizingFlamesAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Check for Agonizing Flames ground effect
    const uint32 SPELL_AGONIZING_FLAMES = 40932;
    
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
    
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        float distance = bot->GetDistance(go);
        if (distance < 8.0f)
        {
            // Move away from flame patch
            float angle = go->GetAngle(bot);
            float moveAngle = angle + M_PI;
            
            Position movePos = go->GetFirstCollisionPosition(12.0f, moveAngle);
            bot->GetMotionMaster()->MovePoint(0, movePos);
            return true;
        }
    }

    return false;
}

bool IllidanEyeBeamAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if in Phase 2 (flying)
    if (!illidan->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
        return false;

    // Check for Eye Beam cast
    const uint32 SPELL_EYE_BLAST = 39908;
    if (!illidan->FindCurrentSpellBySpellId(SPELL_EYE_BLAST))
        return false;

    // Move to safe position away from beam path
    Position centerPos = {676.021f, 305.455f, 353.582f};
    float distanceFromCenter = bot->GetDistance2d(centerPos.GetPositionX(), centerPos.GetPositionY());
    
    if (distanceFromCenter < 30.0f)
    {
        // Move to edge of room
        float angle = centerPos.GetAngle(bot);
        Position movePos = centerPos;
        movePos.m_positionX += cos(angle) * 35.0f;
        movePos.m_positionY += sin(angle) * 35.0f;
        movePos.m_positionZ = bot->GetPositionZ();
        
        bot->GetMotionMaster()->MovePoint(0, movePos);
        return true;
    }

    return false;
}

bool IllidanDarkBarrageInterruptAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if casting Dark Barrage
    const uint32 SPELL_DARK_BARRAGE = 40585;
    if (!illidan->FindCurrentSpellBySpellId(SPELL_DARK_BARRAGE))
        return false;

    // Check if bot has interrupt abilities
    const uint32 SPELL_KICK = 1766;           // Rogue
    const uint32 SPELL_COUNTERSPELL = 2139;   // Mage
    const uint32 SPELL_PUMMEL = 6552;         // Warrior
    const uint32 SPELL_MIND_FREEZE = 47528;   // Death Knight
    
    if (bot->HasSpell(SPELL_KICK) || bot->HasSpell(SPELL_COUNTERSPELL) ||
        bot->HasSpell(SPELL_PUMMEL) || bot->HasSpell(SPELL_MIND_FREEZE))
    {
        botAI->InterruptSpell();
        return true;
    }

    return false;
}

bool IllidanFlameOfAzzinothTankAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // In Phase 2, tanks need to pick up Flames of Azzinoth
    const uint32 NPC_FLAME_OF_AZZINOTH = 22997;
    
    if (!botAI->IsTank(bot))
        return false;

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
    
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FLAME_OF_AZZINOTH)
        {
            // Check if it has a tank
            Unit* currentTarget = unit->GetVictim();
            if (!currentTarget)
            {
                // No target at all, tank it
                if (AI_VALUE(Unit*, "current target") != unit)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                    bot->SetTarget(unit->GetGUID());
                }
                return true;
            }
            
            // Check if current target is a tank player
            Player* targetPlayer = currentTarget->ToPlayer();
            if (!targetPlayer || !botAI->IsTank(targetPlayer))
            {
                if (AI_VALUE(Unit*, "current target") != unit)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                    bot->SetTarget(unit->GetGUID());
                }
                return true;
            }
        }
    }

    return false;
}

bool IllidanShadowDemonAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Shadow Demons must be killed immediately
    const uint32 NPC_SHADOW_DEMON = 23375;
    
    time_t now = time(nullptr);
    if ((now - g_illidan_lastShadowDemonTime[botGuid]) < 2)
        return false;

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
    
    GuidVector npcs = npcsValue->Get();
    
    Unit* closestDemon = nullptr;
    float closestDistance = 50.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SHADOW_DEMON)
        {
            // Shadow Demons fixate on their target
            Unit* demonTarget = unit->GetVictim();
            if (demonTarget == bot)
            {
                // This demon is targeting us - priority!
                closestDemon = unit;
                break;
            }
            
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestDemon = unit;
            }
        }
    }

    if (closestDemon)
    {
        if (AI_VALUE(Unit*, "current target") != closestDemon)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(closestDemon);
            bot->SetTarget(closestDemon->GetGUID());
        }
        g_illidan_lastShadowDemonTime[botGuid] = now;
        return true;
    }

    return false;
}

bool IllidanFlameBurstPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // In Demon Form, spread for Flame Burst
    if (!illidan->HasAura(40506)) // SPELL_DEMON_FORM
        return false;

    // Check distance to other players
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
    
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 5.0f)
        {
            // Spread out
            float angle = member->GetAngle(bot);
            float moveAngle = angle + M_PI;
            
            Position movePos = member->GetFirstCollisionPosition(8.0f, moveAngle);
            bot->GetMotionMaster()->MovePoint(0, movePos);
            return true;
        }
    }

    return false;
}

bool IllidanCageAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Check for nearby cage traps
    const uint32 GO_CAGE_TRAP = 185916;
    
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
    
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || go->GetEntry() != GO_CAGE_TRAP)
            continue;

        float distance = bot->GetDistance(go);
        if (distance < 10.0f)
        {
            // Move away from cage
            float angle = go->GetAngle(bot);
            float moveAngle = angle + M_PI;
            
            Position movePos = go->GetFirstCollisionPosition(15.0f, moveAngle);
            bot->GetMotionMaster()->MovePoint(0, movePos);
            return true;
        }
    }

    return false;
}

bool IllidanEnrageKiteAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if in Phase 5 (Frenzy/Enrage)
    const uint32 SPELL_FRENZY = 40683;
    if (!illidan->HasAura(SPELL_FRENZY))
        return false;

    // Only for the tank
    if (!botAI->IsTank(bot))
        return false;

    // If we're the current target, kite
    if (illidan->GetVictim() == bot)
    {
        // Kite around the room edge
        Position centerPos = {676.021f, 305.455f, 353.582f};
        float angle = centerPos.GetAngle(bot);
        angle += M_PI / 8; // Move in arc
        
        Position movePos = centerPos;
        movePos.m_positionX += cos(angle) * 30.0f;
        movePos.m_positionY += sin(angle) * 30.0f;
        movePos.m_positionZ = bot->GetPositionZ();
        
        bot->GetMotionMaster()->MovePoint(0, movePos);
        return true;
    }

    return false;
}