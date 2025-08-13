#include "ZulAmanActions.h"
#include "AiObjectContext.h"
#include "GameObject.h"
#include "Unit.h"
#include "Value.h"
#include "Item.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Static state maps for proper bot coordination
static std::map<ObjectGuid, uint32> g_nalorakk_lastSwipeAvoid;
static std::map<ObjectGuid, bool> g_nalorakk_bearForm;
static std::map<ObjectGuid, uint32> g_akilzon_lastStormMove;
static std::map<ObjectGuid, ObjectGuid> g_akilzon_stormTarget;
static std::map<ObjectGuid, uint32> g_janalai_lastBombMove;
static std::map<ObjectGuid, uint32> g_halazzi_phase;
static std::map<ObjectGuid, uint32> g_zuljin_phase;
static std::map<ObjectGuid, uint32> g_zuljin_lastAvoidTime;

// --- NALORAKK (Bear) ACTIONS ---

bool NalorakkBrutalSwipeAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = time(nullptr);
    
    // Check cooldown
    if (g_nalorakk_lastSwipeAvoid[botGuid] + 3 > currentTime)
        return false;

    // Brutal Swipe hits in a frontal cone - tanks should face him away
    if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
    {
        // Move behind if in front
        float angle = bot->GetAngle(nalorakk);
        float myAngle = nalorakk->GetAngle(bot);
        
        if (fabs(myAngle) < M_PI / 3) // Within frontal cone
        {
            float moveAngle = nalorakk->GetOrientation() + M_PI;
            Position movePos = nalorakk->GetFirstCollisionPosition(8.0f, moveAngle);
            
            if (bot->GetMotionMaster())
            {
                bot->GetMotionMaster()->MovePoint(0, movePos);
                g_nalorakk_lastSwipeAvoid[botGuid] = currentTime;
                return true;
            }
        }
    }
    
    return false;
}

bool NalorakkSurgePositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    // During Surge, non-tanks should maintain distance
    const uint32 SPELL_SURGE = 42402;
    
    if (nalorakk->FindCurrentSpellBySpellId(SPELL_SURGE))
    {
        if (!botAI->IsTank(bot))
        {
            float distance = bot->GetDistance(nalorakk);
            if (distance < 15.0f)
            {
                float moveAngle = bot->GetAngle(nalorakk) + M_PI;
                Position movePos = bot->GetFirstCollisionPosition(20.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool NalorakkBearFormPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    const uint32 SPELL_BEARFORM = 42377;
    
    // Check if in bear form
    bool isBearForm = nalorakk->HasAura(SPELL_BEARFORM);
    
    if (isBearForm != g_nalorakk_bearForm[botGuid])
    {
        g_nalorakk_bearForm[botGuid] = isBearForm;
        
        // In bear form, spread out for Deafening Roar
        if (isBearForm && !botAI->IsTank(bot))
        {
            // Spread from other players
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
                    float moveAngle = bot->GetAngle(member) + M_PI;
                    Position movePos = bot->GetFirstCollisionPosition(8.0f, moveAngle);
                    
                    if (bot->GetMotionMaster())
                    {
                        bot->GetMotionMaster()->MovePoint(0, movePos);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// --- AKIL'ZON (Eagle) ACTIONS ---

bool AkilzonElectricalStormAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akilzon");
    if (!akilzon || !akilzon->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = time(nullptr);
    
    const uint32 SPELL_ELECTRICAL_STORM = 43648;
    const uint32 SPELL_ELECTRICAL_STORM_AREA = 44007; // Safe area spell
    
    // Check if electrical storm is active
    if (akilzon->FindCurrentSpellBySpellId(SPELL_ELECTRICAL_STORM))
    {
        // Check cooldown
        if (g_akilzon_lastStormMove[botGuid] + 2 > currentTime)
            return false;
            
        // Check if we have the safe area buff
        if (bot->HasAura(SPELL_ELECTRICAL_STORM_AREA))
            return false; // Already safe
            
        // Find the storm target/center
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member)
                continue;
                
            // Check if this player is lifted (storm center)
            if (member->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
            {
                // Move to under the lifted player
                Position targetPos = member->GetPosition();
                targetPos.m_positionZ = bot->GetPositionZ(); // Stay on ground
                
                float distance = bot->GetDistance2d(targetPos.m_positionX, targetPos.m_positionY);
                if (distance > 3.0f)
                {
                    if (bot->GetMotionMaster())
                    {
                        bot->GetMotionMaster()->MovePoint(0, targetPos);
                        g_akilzon_lastStormMove[botGuid] = currentTime;
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool AkilzonStaticDisruptionAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_STATIC_DISRUPTION = 43622;
    
    // Check if we have static disruption debuff
    if (bot->HasAura(SPELL_STATIC_DISRUPTION))
    {
        // Spread from other players to avoid chain damage
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
            if (distance < 8.0f)
            {
                float moveAngle = bot->GetAngle(member) + M_PI;
                Position movePos = bot->GetFirstCollisionPosition(12.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool AkilzonSoaringEagleTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_SOARING_EAGLE = 24858;
    
    // Find eagles
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_SOARING_EAGLE)
        {
            // Switch to eagle if not current target
            if (AI_VALUE(Unit*, "current target") != unit)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                bot->SetTarget(unit->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

// --- JAN'ALAI (Dragonhawk) ACTIONS ---

bool JanalaiFireBombAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = time(nullptr);
    
    // Check cooldown
    if (g_janalai_lastBombMove[botGuid] + 2 > currentTime)
        return false;

    const uint32 NPC_FIRE_BOMB = 23920;
    
    // Find fire bombs
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        if (unit->GetEntry() == NPC_FIRE_BOMB)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 8.0f)
            {
                // Move away from fire bomb
                float moveAngle = bot->GetAngle(unit) + M_PI;
                Position movePos = bot->GetFirstCollisionPosition(12.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    g_janalai_lastBombMove[botGuid] = currentTime;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool JanalaiHatcherTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_AMANI_HATCHER = 23818;
    
    // Find hatchers - high priority
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_AMANI_HATCHER)
        {
            // Switch to hatcher immediately
            if (AI_VALUE(Unit*, "current target") != unit)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                bot->SetTarget(unit->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

bool JanalaiHatchlingTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_HATCHLING = 23598; // Amani Dragonhawk Hatchling
    
    // AoE classes should handle hatchlings
    if (botAI->IsRanged(bot))
    {
        // Find hatchlings
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
            return false;
            
        GuidVector npcs = npcsValue->Get();
        uint32 hatchlingCount = 0;
        Unit* nearestHatchling = nullptr;
        float nearestDistance = 100.0f;
        
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == NPC_HATCHLING)
            {
                hatchlingCount++;
                float distance = bot->GetDistance(unit);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestHatchling = unit;
                }
            }
        }
        
        // Switch to hatchlings if many are present
        if (hatchlingCount > 3 && nearestHatchling)
        {
            if (AI_VALUE(Unit*, "current target") != nearestHatchling)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestHatchling);
                bot->SetTarget(nearestHatchling->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

bool JanalaiFireWallAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai || !janalai->IsAlive())
        return false;

    const uint32 SPELL_FIRE_WALL = 43113;
    
    // Check if fire wall is being cast
    if (janalai->FindCurrentSpellBySpellId(SPELL_FIRE_WALL))
    {
        // Move to center of platform
        Position centerPos = janalai->GetPosition();
        float distance = bot->GetDistance2d(centerPos.m_positionX, centerPos.m_positionY);
        
        if (distance > 15.0f)
        {
            if (bot->GetMotionMaster())
            {
                bot->GetMotionMaster()->MovePoint(0, centerPos);
                return true;
            }
        }
    }
    
    return false;
}

// --- HALAZZI (Lynx) ACTIONS ---

bool HalazziLynxTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_SPIRIT_LYNX = 24143;
    
    // Find spirit lynx - high priority during split phases
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_SPIRIT_LYNX)
        {
            // Lynx is priority target during split phases
            if (AI_VALUE(Unit*, "current target") != unit)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                bot->SetTarget(unit->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

bool HalazziTotemTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_TOTEM = 24224;
    
    // Find totems - highest priority
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_TOTEM)
        {
            // Totem is highest priority
            if (AI_VALUE(Unit*, "current target") != unit)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                bot->SetTarget(unit->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

bool HalazziSaberLashPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi || !halazzi->IsAlive())
        return false;

    const uint32 SPELL_SABER_LASH = 43267;
    
    // Saber Lash splits damage among targets in front
    if (halazzi->FindCurrentSpellBySpellId(SPELL_SABER_LASH))
    {
        // Tanks should stack in front, others behind
        if (botAI->IsTank(bot))
        {
            // Stack with other tanks
            Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
            if (!membersValue)
                return false;
                
            GuidVector members = membersValue->Get();
            
            for (auto& guid : members)
            {
                Player* member = ObjectAccessor::FindPlayer(guid);
                if (!member || member == bot)
                    continue;
                    
                if (botAI->IsTank(member))
                {
                    float distance = bot->GetDistance(member);
                    if (distance > 3.0f)
                    {
                        Position movePos = member->GetPosition();
                        if (bot->GetMotionMaster())
                        {
                            bot->GetMotionMaster()->MovePoint(0, movePos);
                            return true;
                        }
                    }
                }
            }
        }
        else
        {
            // Non-tanks move behind
            float myAngle = halazzi->GetAngle(bot);
            if (fabs(myAngle) < M_PI / 3) // Within frontal cone
            {
                float moveAngle = halazzi->GetOrientation() + M_PI;
                Position movePos = halazzi->GetFirstCollisionPosition(8.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    return true;
                }
            }
        }
    }
    
    return false;
}

// --- HEX LORD MALACRASS ACTIONS ---

bool HexLordDrainPowerInterruptAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* hexlord = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!hexlord || !hexlord->IsAlive())
        return false;

    const uint32 SPELL_DRAIN_POWER = 44131;
    
    // Check if Drain Power is being cast
    if (hexlord->FindCurrentSpellBySpellId(SPELL_DRAIN_POWER))
    {
        // Check if bot has interrupt abilities
        const uint32 SPELL_KICK = 1766;           // Rogue
        const uint32 SPELL_COUNTERSPELL = 2139;   // Mage
        const uint32 SPELL_PUMMEL = 6552;         // Warrior
        const uint32 SPELL_MIND_FREEZE = 47528;   // Death Knight
        const uint32 SPELL_WIND_SHEAR = 57994;    // Shaman
        
        if (bot->HasSpell(SPELL_KICK) || bot->HasSpell(SPELL_COUNTERSPELL) ||
            bot->HasSpell(SPELL_PUMMEL) || bot->HasSpell(SPELL_MIND_FREEZE) ||
            bot->HasSpell(SPELL_WIND_SHEAR))
        {
            botAI->InterruptSpell();
            return true;
        }
    }
    
    return false;
}

bool HexLordAddTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Priority: Thurg > Alyson Antille > Slither > Lord Raadan
    const uint32 priorityList[] = {
        24241, // Thurg (Healer - highest priority)
        24240, // Alyson Antille (Mage)
        24242, // Slither (Rogue)
        24243  // Lord Raadan (Druid)
    };
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (uint32 npcId : priorityList)
    {
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == npcId)
            {
                if (AI_VALUE(Unit*, "current target") != unit)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                    bot->SetTarget(unit->GetGUID());
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool HexLordSpiritBolleyAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* hexlord = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!hexlord || !hexlord->IsAlive())
        return false;

    const uint32 SPELL_SPIRIT_BOLTS = 43383;
    
    // Check if Spirit Bolts is being cast
    if (hexlord->FindCurrentSpellBySpellId(SPELL_SPIRIT_BOLTS))
    {
        // Spread to reduce damage
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
            if (distance < 6.0f)
            {
                float moveAngle = bot->GetAngle(member) + M_PI;
                Position movePos = bot->GetFirstCollisionPosition(10.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    return true;
                }
            }
        }
    }
    
    return false;
}

// --- ZUL'JIN ACTIONS ---

bool ZuljinGrievousThrowHealAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_GRIEVOUS_THROW = 43093;
    
    // Check if anyone has Grievous Throw debuff
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || !member->IsAlive())
            continue;
            
        if (member->HasAura(SPELL_GRIEVOUS_THROW))
        {
            // Heal to full to remove debuff
            if (member->GetHealthPct() < 100.0f)
            {
                if (botAI->IsHeal(bot))
                {
                    // Prioritize healing this target
                    if (AI_VALUE(Unit*, "current target") != member)
                    {
                        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(member);
                        bot->SetTarget(member->GetGUID());
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool ZuljinCreepingParalysisAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_CREEPING_PARALYSIS = 43095;
    
    // Move to dispel Creeping Paralysis
    if (bot->HasAura(SPELL_CREEPING_PARALYSIS))
    {
        // Simply move to remove stacks
        float moveAngle = bot->GetOrientation() + frand(-M_PI/4, M_PI/4);
        Position movePos = bot->GetFirstCollisionPosition(5.0f, moveAngle);
        
        if (bot->GetMotionMaster())
        {
            bot->GetMotionMaster()->MovePoint(0, movePos);
            return true;
        }
    }
    
    return false;
}

bool ZuljinFeatherVortexTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_FEATHER_VORTEX = 24136;
    
    // Find and target feather vortexes
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_FEATHER_VORTEX)
        {
            if (AI_VALUE(Unit*, "current target") != unit)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                bot->SetTarget(unit->GetGUID());
                return true;
            }
        }
    }
    
    return false;
}

bool ZuljinCycloneAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = time(nullptr);
    
    // Check cooldown
    if (g_zuljin_lastAvoidTime[botGuid] + 2 > currentTime)
        return false;

    const uint32 NPC_CYCLONE = 24136; // Feather Vortex acts as cyclone
    
    // Find cyclones and avoid them
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        if (unit->GetEntry() == NPC_CYCLONE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)
            {
                // Move away from cyclone
                float moveAngle = bot->GetAngle(unit) + M_PI;
                Position movePos = bot->GetFirstCollisionPosition(15.0f, moveAngle);
                
                if (bot->GetMotionMaster())
                {
                    bot->GetMotionMaster()->MovePoint(0, movePos);
                    g_zuljin_lastAvoidTime[botGuid] = currentTime;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool ZuljinFlameColumnAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = time(nullptr);
    
    // Check cooldown
    if (g_zuljin_lastAvoidTime[botGuid] + 2 > currentTime)
        return false;

    const uint32 SPELL_FLAME_COLUMN = 43215;
    
    // Check if we're in a flame column area
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->IsAlive())
        return false;

    // During fire phase, constantly move to avoid flame columns
    if (g_zuljin_phase[botGuid] == 3) // Fire phase
    {
        // Keep moving in a pattern
        float moveAngle = bot->GetOrientation() + M_PI/2;
        Position movePos = bot->GetFirstCollisionPosition(10.0f, moveAngle);
        
        if (bot->GetMotionMaster())
        {
            bot->GetMotionMaster()->MovePoint(0, movePos);
            g_zuljin_lastAvoidTime[botGuid] = currentTime;
            return true;
        }
    }
    
    return false;
}