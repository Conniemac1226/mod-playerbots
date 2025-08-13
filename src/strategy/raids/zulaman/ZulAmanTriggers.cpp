#include "ZulAmanTriggers.h"
#include "AiObjectContext.h"
#include "GameObject.h"
#include "Unit.h"
#include "Value.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// --- NALORAKK (Bear) TRIGGERS ---

bool NalorakkBrutalSwipeTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    const uint32 SPELL_BRUTALSWIPE = 42384;
    
    // Check if casting or about to cast Brutal Swipe
    if (nalorakk->FindCurrentSpellBySpellId(SPELL_BRUTALSWIPE))
        return true;
    
    // Check if in melee range and in front
    if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
    {
        float distance = bot->GetDistance(nalorakk);
        float myAngle = nalorakk->GetAngle(bot);
        
        if (distance < 8.0f && fabs(myAngle) < M_PI / 3)
            return true;
    }
    
    return false;
}

bool NalorakkSurgeTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    const uint32 SPELL_SURGE = 42402;
    
    // Check if Surge is active or being cast
    return nalorakk->FindCurrentSpellBySpellId(SPELL_SURGE);
}

bool NalorakkBearFormTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk || !nalorakk->IsAlive())
        return false;

    const uint32 SPELL_BEARFORM = 42377;
    
    // Check if in bear form
    return nalorakk->HasAura(SPELL_BEARFORM);
}

// --- AKIL'ZON (Eagle) TRIGGERS ---

bool AkilzonElectricalStormTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akilzon");
    if (!akilzon || !akilzon->IsAlive())
        return false;

    const uint32 SPELL_ELECTRICAL_STORM = 43648;
    const uint32 SPELL_ELECTRICAL_STORM_AREA = 44007;
    
    // Check if storm is active and we're not in safe area
    if (akilzon->FindCurrentSpellBySpellId(SPELL_ELECTRICAL_STORM))
    {
        if (!bot->HasAura(SPELL_ELECTRICAL_STORM_AREA))
            return true;
    }
    
    // Check if any player is lifted
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member)
            continue;
            
        if (member->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
            return true;
    }
    
    return false;
}

bool AkilzonStaticDisruptionTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_STATIC_DISRUPTION = 43622;
    
    // Check if we have the debuff and need to spread
    if (bot->HasAura(SPELL_STATIC_DISRUPTION))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;
                
            if (bot->GetDistance(member) < 8.0f)
                return true;
        }
    }
    
    return false;
}

bool AkilzonSoaringEagleTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_SOARING_EAGLE = 24858;
    
    // Check for eagles
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
            return true;
    }
    
    return false;
}

// --- JAN'ALAI (Dragonhawk) TRIGGERS ---

bool JanalaiFireBombTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_FIRE_BOMB = 23920;
    
    // Check for fire bombs nearby
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
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }
    
    return false;
}

bool JanalaiHatcherTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_AMANI_HATCHER = 23818;
    
    // Check for hatchers
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
            return true;
    }
    
    return false;
}

bool JanalaiHatchlingTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_HATCHLING = 23598;
    
    // Check for multiple hatchlings
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    uint32 hatchlingCount = 0;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HATCHLING)
        {
            hatchlingCount++;
            if (hatchlingCount > 3)
                return true;
        }
    }
    
    return false;
}

bool JanalaiFireWallTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai || !janalai->IsAlive())
        return false;

    const uint32 SPELL_FIRE_WALL = 43113;
    
    // Check if casting Fire Wall
    return janalai->FindCurrentSpellBySpellId(SPELL_FIRE_WALL);
}

// --- HALAZZI (Lynx) TRIGGERS ---

bool HalazziLynxTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_SPIRIT_LYNX = 24143;
    
    // Check for spirit lynx
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
            return true;
    }
    
    return false;
}

bool HalazziTotemTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_TOTEM = 24224;
    
    // Check for totems
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
            return true;
    }
    
    return false;
}

bool HalazziSaberLashTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi || !halazzi->IsAlive())
        return false;

    const uint32 SPELL_SABER_LASH = 43267;
    
    // Check if casting Saber Lash
    return halazzi->FindCurrentSpellBySpellId(SPELL_SABER_LASH);
}

// --- HEX LORD MALACRASS TRIGGERS ---

bool HexLordDrainPowerTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* hexlord = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!hexlord || !hexlord->IsAlive())
        return false;

    const uint32 SPELL_DRAIN_POWER = 44131;
    
    // Check if casting Drain Power - needs interrupt
    if (hexlord->FindCurrentSpellBySpellId(SPELL_DRAIN_POWER))
    {
        // Only trigger if we can interrupt
        const uint32 SPELL_KICK = 1766;
        const uint32 SPELL_COUNTERSPELL = 2139;
        const uint32 SPELL_PUMMEL = 6552;
        const uint32 SPELL_MIND_FREEZE = 47528;
        const uint32 SPELL_WIND_SHEAR = 57994;
        
        if (bot->HasSpell(SPELL_KICK) || bot->HasSpell(SPELL_COUNTERSPELL) ||
            bot->HasSpell(SPELL_PUMMEL) || bot->HasSpell(SPELL_MIND_FREEZE) ||
            bot->HasSpell(SPELL_WIND_SHEAR))
            return true;
    }
    
    return false;
}

bool HexLordAddTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for adds
    const uint32 addIds[] = {24241, 24240, 24242, 24243};
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (uint32 addId : addIds)
    {
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == addId)
                return true;
        }
    }
    
    return false;
}

bool HexLordSpiritBolleyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* hexlord = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!hexlord || !hexlord->IsAlive())
        return false;

    const uint32 SPELL_SPIRIT_BOLTS = 43383;
    
    // Check if casting Spirit Bolts
    if (hexlord->FindCurrentSpellBySpellId(SPELL_SPIRIT_BOLTS))
    {
        // Check if too close to others
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;
                
            if (bot->GetDistance(member) < 6.0f)
                return true;
        }
    }
    
    return false;
}

// --- ZUL'JIN TRIGGERS ---

bool ZuljinGrievousThrowTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_GRIEVOUS_THROW = 43093;
    
    // Check if anyone needs healing from Grievous Throw
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || !member->IsAlive())
            continue;
            
        if (member->HasAura(SPELL_GRIEVOUS_THROW) && member->GetHealthPct() < 100.0f)
        {
            if (botAI->IsHeal(bot))
                return true;
        }
    }
    
    return false;
}

bool ZuljinCreepingParalysisTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_CREEPING_PARALYSIS = 43095;
    
    // Check if we have Creeping Paralysis and need to move
    return bot->HasAura(SPELL_CREEPING_PARALYSIS);
}

bool ZuljinFeatherVortexTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_FEATHER_VORTEX = 24136;
    
    // Check for feather vortexes
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
            return true;
    }
    
    return false;
}

bool ZuljinCycloneTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_CYCLONE = 24136;
    
    // Check for cyclones nearby
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
            if (bot->GetDistance(unit) < 10.0f)
                return true;
        }
    }
    
    return false;
}

bool ZuljinFlameColumnTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->IsAlive())
        return false;

    // During Dragonhawk phase, flame columns spawn
    // Check if Zul'jin health indicates fire phase (60-40%)
    float healthPct = zuljin->GetHealthPct();
    if (healthPct <= 60.0f && healthPct > 40.0f)
        return true;
    
    return false;
}