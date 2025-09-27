#include "SerpentshrineTriggers.h"
#include "SerpentshrineActions.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Value.h"

bool HydrossMarkOfHydrossTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    if (bot->HasAura(SPELL_VILE_SLUDGE))
    {
        return true;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    return bot->HasAura(SPELL_MARK_OF_HYDROSS1) ||
           bot->HasAura(SPELL_MARK_OF_HYDROSS2) ||
           bot->HasAura(SPELL_MARK_OF_HYDROSS3) ||
           bot->HasAura(SPELL_MARK_OF_HYDROSS4) ||
           bot->HasAura(SPELL_MARK_OF_HYDROSS5) ||
           bot->HasAura(SPELL_MARK_OF_HYDROSS6);
}

bool HydrossMarkOfCorruptionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    return bot->HasAura(SPELL_MARK_OF_CORRUPTION1) ||
           bot->HasAura(SPELL_MARK_OF_CORRUPTION2) ||
           bot->HasAura(SPELL_MARK_OF_CORRUPTION3) ||
           bot->HasAura(SPELL_MARK_OF_CORRUPTION4) ||
           bot->HasAura(SPELL_MARK_OF_CORRUPTION5) ||
           bot->HasAura(SPELL_MARK_OF_CORRUPTION6);
}

bool HydrossWaterTombTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    if (AI_VALUE(float, "hydross water tomb spread distance") > 0.0f)
    {
        return true;
    }

    // Check for cast or if boss is in water phase and about to cast
    if (!boss->HasAura(SPELL_HYDROSS_CORRUPTION))
    {
        // Immediate reaction to cast
        if (boss->FindCurrentSpellBySpellId(SPELL_WATER_TOMB))
        {
            Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
            if (!membersValue)
            {
                return false;
            }
            GuidVector members = membersValue->Get();
            for (ObjectGuid const& member : members)
            {
                Unit* player = botAI->GetUnit(member);
                if (player && player != bot && player->IsAlive())
                {
                    float distance = bot->GetDistance(player);
                    if (distance < 10.0f) // Slightly larger range for faster reaction
                    {
                        return true;
                    }
                }
            }
        }
        // Also check if boss is casting (pre-emptive detection)
        else if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
            if (!membersValue)
            {
                return false;
            }
            GuidVector members = membersValue->Get();
            for (ObjectGuid const& member : members)
            {
                Unit* player = botAI->GetUnit(member);
                if (player && player != bot && player->IsAlive())
                {
                    float distance = bot->GetDistance(player);
                    if (distance < 6.0f) // Very close = urgent spread
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool HydrossVileSludgeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    if (AI_VALUE(float, "hydross vile sludge spread distance") > 0.0f)
    {
        return true;
    }

    if (boss->HasAura(SPELL_HYDROSS_CORRUPTION))
    {
        float distance = bot->GetDistance(boss);
        
        // Immediate reaction to cast
        if (boss->FindCurrentSpellBySpellId(SPELL_VILE_SLUDGE))
        {
            return distance < 20.0f; // Larger range for faster reaction
        }
        // Pre-emptive check when boss is casting anything in poison form
        else if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            return distance < 12.0f; // Start moving early if too close
        }
    }

    return false;
}

bool HydrossAddsTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_SPAWN_OF_HYDROSS || 
            unit->GetEntry() == NPC_TAINTED_SPAWN_OF_HYDROSS)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 40.0f)
            {
                return true;
            }
        }
    }

    return false;
}

bool HydrossTransitionNeededTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint8 stacks = AI_VALUE(uint8, "hydross mark stacks");
    bool transitionNeeded = AI_VALUE(bool, "hydross transition needed");

    return stacks >= 4 && transitionNeeded;
}

bool HydrossTankPositionTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    
    float desiredX, desiredY;
    
    if (hasCorruption)
    {
        desiredX = -29.0f;
        desiredY = -923.0f;
    }
    else
    {
        desiredX = 71.0f;
        desiredY = -883.0f;
    }
    
    Position desiredPos(desiredX, desiredY, bot->GetPositionZ());
    float distance = bot->GetDistance(desiredPos);
    
    return distance > 5.0f;
}

// The Lurker Below Triggers
bool LurkerSpoutTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    if (AI_VALUE(float, "lurker geyser spread distance") > 0.0f)
    {
        return true;
    }

    // Detect spout cast or visual
    return boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) || 
           boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_1) ||
           boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_2) ||
           (boss->HasUnitState(UNIT_STATE_CASTING) && !bot->IsInWater());
}

bool LurkerWhirlTrigger::IsActive()
{
    if (!bot || !botAI || !botAI->IsMelee(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for whirl cast
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_WHIRL) || boss->HasAura(SPELL_LURKER_WHIRL))
    {
        float distance = bot->GetDistance(boss);
        return distance < 8.0f;
    }

    return false;
}

bool LurkerGeyserTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for geyser cast and proximity to allies
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_GEYSER))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 10.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool LurkerAddsTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only check for adds when boss is submerged
    if (!boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_COILFANG_GUARDIAN || unit->GetEntry() == NPC_COILFANG_AMBUSHER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 50.0f)
            {
                return true;
            }
        }
    }

    return false;
}

bool LurkerPositionTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Don't reposition during spout or when submerged
    if (boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }

    float desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    float currentDistance = bot->GetDistance(boss);
    
    return fabs(currentDistance - desiredDistance) > 3.0f;
}

// Leotheras the Blind Triggers
bool LeotherasWhirlwindTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint32 holdUntil = AI_VALUE(uint32, "leotheras whirlwind hold until");
    if (holdUntil > getMSTime())
    {
        return true;
    }

    // Check for whirlwind cast
    if (boss->FindCurrentSpellBySpellId(SPELL_LEOTHERAS_WHIRLWIND) || boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND))
    {
        float distance = bot->GetDistance(boss);
        return distance < 15.0f;
    }

    return false;
}

bool LeotherasChaosBlastTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is in demon form and near other players
    if (boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 8.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool LeotherasInnerDemonTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for Inner Demon summoned for this bot
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_INNER_DEMON)
        {
            if (unit->ToCreature() && unit->ToCreature()->GetSummonerGUID() == bot->GetGUID())
            {
                return true;
            }
        }
    }

    return false;
}

bool LeotherasShadowTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Shadow spawns at 15% health
    if (boss->GetHealthPct() <= 15.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_SHADOW_OF_LEOTHERAS)
            {
                return true;
            }
        }
    }

    return false;
}

bool LeotherasPositionTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Don't reposition during whirlwind
    if (boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND))
    {
        return false;
    }

    float desiredDistance;
    if (boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS))
    {
        desiredDistance = botAI->IsRanged(bot) ? 30.0f : 35.0f;
    }
    else
    {
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }

    float currentDistance = bot->GetDistance(boss);
    return fabs(currentDistance - desiredDistance) > 3.0f;
}

// Fathom-Lord Karathress Triggers
bool KarathressCataclysmicBoltTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
   if (!boss || !boss->IsAlive() || !boss->IsInCombat())
   {
       return false;
   }

    if (AI_VALUE(float, "karathress sear nova spread distance") > 0.0f)
    {
        return true;
    }

    // Cataclysmic Bolt targets mana users
    if (bot->getPowerType() == POWER_MANA && boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_CATACLYSMIC_BOLT))
    {
        return true;
    }

    return false;
}

bool KarathressSearNovaTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Sear Nova melee AoE
    if (boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_SEAR_NOVA) || boss->HasUnitState(UNIT_STATE_CASTING))
    {
        float distance = bot->GetDistance(boss);
        return distance < 10.0f;
    }

    return false;
}

bool KarathressAdvisorsTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        // Check for any alive advisor
        if (unit->GetEntry() == NPC_FATHOM_GUARD_SHARKKIS ||
            unit->GetEntry() == NPC_FATHOM_GUARD_TIDALVESS ||
            unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS)
        {
            return true;
        }
    }

    return false;
}

bool KarathressTidalSurgeTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check for Caribdis casting Tidal Surge
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_KARATHRESS_TIDAL_SURGE))
            {
                float distance = bot->GetDistance(unit);
                if (distance < 10.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool KarathressTotemsTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        // Check for totems that need to be killed
        if (unit->GetEntry() == NPC_SPITFIRE_TOTEM ||
            unit->GetEntry() == NPC_GREATER_EARTHBIND_TOTEM)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 30.0f)
            {
                return true;
            }
        }
    }

    return false;
}

// Morogrim Tidewalker Triggers
bool MorogrimTidalWaveTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting Tidal Wave and bot is in front
    if (boss->FindCurrentSpellBySpellId(SPELL_MOROGRIM_TIDAL_WAVE) || boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // Check if bot is in front of boss
        float angle = boss->GetAngle(bot);
        float facing = boss->GetOrientation();
        float diff = fabs(angle - facing);
        
        // Normalize angle difference
        if (diff > M_PI)
            diff = 2 * M_PI - diff;
            
        // In front cone (90 degrees)
        if (diff < M_PI / 4)
        {
            float distance = bot->GetDistance(boss);
            return distance < 20.0f;
        }
    }

    return false;
}

bool MorogrimWateryGraveTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is above 25% and about to cast Watery Grave
    if (boss->GetHealthPct() > 25.0f)
    {
        // Check for Watery Grave debuff on self
        if (bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_1) || 
            bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_2) || 
            bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_3) || 
            bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_4))
        {
            return true;
        }
    }

    return false;
}

bool MorogrimMurlocsTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for murlocs
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        // Tidewalker Lurker
        if (unit->GetEntry() == NPC_TIDEWALKER_LURKER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 40.0f)
            {
                return true;
            }
        }
    }

    return false;
}

bool MorogrimGlobulesTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only in phase 2 (below 25%)
    if (boss->GetHealthPct() <= 25.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            // Water Globule
            if (unit->GetEntry() == NPC_WATER_GLOBULE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 15.0f) // They fixate and chase
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool MorogrimPositionTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Different positioning for phase 2
    float desiredDistance;
    if (boss->GetHealthPct() <= 25.0f)
    {
        // Phase 2 - need more room for globules
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 30.0f;
    }
    else
    {
        // Phase 1 - stack behind boss
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 15.0f;
    }

    float currentDistance = bot->GetDistance(boss);
    return fabs(currentDistance - desiredDistance) > 5.0f;
}

// Lady Vashj Triggers
bool VashjShockBlastTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for Shock Blast cast (frontal cone)
    if (boss->FindCurrentSpellBySpellId(SPELL_VASHJ_SHOCK_BLAST))
    {
        // Check if bot is in front
        float angle = boss->GetAngle(bot);
        float facing = boss->GetOrientation();
        float diff = fabs(angle - facing);
        
        // Normalize angle difference
        if (diff > M_PI)
            diff = 2 * M_PI - diff;
            
        // In front cone (90 degrees)
        if (diff < M_PI / 4)
        {
            float distance = bot->GetDistance(boss);
            return distance < 20.0f;
        }
    }

    return false;
}

bool VashjStaticChargeTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    if (AI_VALUE(float, "vashj static charge spread distance") > 0.0f)
    {
        return true;
    }

    // Check if bot has Static Charge and needs to spread
    if (bot->HasAura(SPELL_VASHJ_STATIC_CHARGE))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 10.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjEntangleTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check if bot is entangled
    return bot->HasAura(SPELL_VASHJ_ENTANGLE);
}

bool VashjEnchantedElementalTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check for Enchanted Elementals
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_ENCHANTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 50.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjTaintedElementalTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check for Tainted Elementals
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TAINTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 50.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjCoilfangEliteTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check for Coilfang Elites
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_ELITE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 50.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjCoilfangStriderTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check for Coilfang Striders
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_STRIDER)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 50.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjSporebatTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 3 - check for Sporebats
    if (!boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER) && boss->GetHealthPct() <= 50.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TOXIC_SPOREBAT)
            {
                float distance = bot->GetDistance(unit);
                if (distance < 40.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool VashjPositionTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float desiredDistance;
    
    // Phase 2 - spread out for adds
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        desiredDistance = botAI->IsRanged(bot) ? 30.0f : 20.0f;
    }
    // Phase 1 and 3 - standard positioning  
    else
    {
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }
    
    float currentDistance = bot->GetDistance(boss);
    return fabs(currentDistance - desiredDistance) > 5.0f;
}

bool VashjTaintedCoreTrigger::IsActive()
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check if bot has tainted core
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        return bot->HasItemCount(ITEM_TAINTED_CORE, 1);
    }

    return false;
}
