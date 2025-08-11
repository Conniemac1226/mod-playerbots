#include "SteamvaultActions.h"
#include "SteamvaultTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "AttackersValue.h"
#include "Playerbots.h"
#include "Value.h"

// Use NPC and spell IDs from SteamvaultTriggers.h

// Hydromancer Thespia Actions
bool AvoidLightningCloudAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydromancer thespia");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Lightning Cloud - boss_hydromancer_thespia.cpp:64-66
    // Boss casts Lightning Cloud on random target location
    // Must evacuate the area immediately!
    if (boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_CLOUD))
    {
        // Check if we're the target or near target area
        float distance = bot->GetDistance(boss);
        if (distance < 25.0f) // Lightning Cloud has large AoE
        {
            // EMERGENCY: Move to safe distance
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            
            // Stop casting and move immediately
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AvoidLightningCloudAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = bot->FindNearestCreature(NPC_HYDROMANCER_THESPIA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
        
    return boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_CLOUD) && bot->GetDistance(boss) < 25.0f;
}

bool DispelLungBurstAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to dispel Lung Burst from self or allies
    if (bot->HasAura(SPELL_LUNG_BURST))
    {
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, bot, false))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }
    return false;
}

bool DispelLungBurstAction::isUseful()
{
    return AI_VALUE(bool, "thespia lung burst");
}

bool AttackWaterElementalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find water elementals using proven pattern
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* elemental = nullptr;
    float closestDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                elemental = unit;
                closestDistance = distance;
            }
        }
    }

    if (elemental)
    {
        return Attack(elemental);
    }

    return false;
}

bool AttackWaterElementalAction::isUseful()
{
    return AI_VALUE(bool, "thespia water elemental active");
}

// Mekgineer Steamrigger Actions
bool DispelShrinkRayAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to dispel shrink ray
    if (bot->HasAura(SPELL_SUPER_SHRINK_RAY))
    {
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, bot, false))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }
    return false;
}

bool DispelShrinkRayAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger shrink ray");
}

bool AvoidSawBladeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mekgineer steamrigger");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Saw Blade - boss_mekgineer_steamrigger.cpp:74-81
    // Thrown at random target or victim if no valid target
    // Move immediately when cast starts!
    if (boss->FindCurrentSpellBySpellId(SPELL_SAW_BLADE))
    {
        // Check if we're the target
        if (boss->GetTarget() == bot->GetGUID() || boss->GetVictim() == bot)
        {
            // EMERGENCY: Move perpendicular to avoid the blade path
            float angle = boss->GetAngle(bot);
            float newAngle = angle + (frand(0, 1) > 0.5f ? M_PI / 2 : -M_PI / 2); // Random side
            
            float moveX = bot->GetPositionX() + cos(newAngle) * 15.0f;
            float moveY = bot->GetPositionY() + sin(newAngle) * 15.0f;
            float moveZ = bot->GetPositionZ();

            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool AvoidSawBladeAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger saw blade");
}

bool RemoveElectrifiedNetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to remove net with escape abilities
    if (bot->HasAura(SPELL_ELECTRIFIED_NET))
    {
        // Try PvP trinket or escape abilities
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "escape");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, bot, false))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }
    return false;
}

bool RemoveElectrifiedNetAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger electrified net");
}

bool AttackSteamriggerMechanicAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find mechanics using proven pattern
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* mechanic = nullptr;
    float closestDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_STEAMRIGGER_MECHANIC && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                mechanic = unit;
                closestDistance = distance;
            }
        }
    }

    if (mechanic)
    {
        return Attack(mechanic);
    }

    return false;
}

bool AttackSteamriggerMechanicAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger mechanic active");
}

// Warlord Kalithresh Actions
bool StopCastingSpellReflectionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "warlord kalithresh");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Spell Reflection - boss_warlord_kalithresh.cpp:79-82
    // Boss casts spell reflection every 20-36 seconds
    // CRITICAL: Stop ALL spellcasting immediately!
    if (boss->HasAura(SPELL_SPELL_REFLECTION) || boss->FindCurrentSpellBySpellId(SPELL_SPELL_REFLECTION))
    {
        // Interrupt any current cast
        if (bot->HasUnitState(UNIT_STATE_CASTING))
        {
            bot->InterruptNonMeleeSpells(true);
        }
        
        // Clear target to prevent auto-cast
        if (botAI->IsCaster(bot))
        {
            bot->AttackStop();
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        }
        
        return true;
    }

    return false;
}

bool StopCastingSpellReflectionAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh spell reflection");
}

bool HealImpaleTargetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Prioritize healing impaled targets
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* impaleTarget = nullptr;
    float lowestHealth = 100.0f;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_IMPALE))
        {
            float healthPct = member->GetHealthPct();
            if (healthPct < lowestHealth)
            {
                impaleTarget = member;
                lowestHealth = healthPct;
            }
        }
    }

    if (impaleTarget)
    {
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, impaleTarget, false))
                {
                    return botAI->CastSpell(spellId, impaleTarget);
                }
            }
        }
    }

    return false;
}

bool HealImpaleTargetAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh impale");
}

bool AttackNagaDistillerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find active Naga Distillers using proven pattern
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* distiller = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        // Only attack distillers that are selectable (active)
        if (unit->GetEntry() == NPC_NAGA_DISTILLER && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                distiller = unit;
                closestDistance = distance;
            }
        }
    }

    if (distiller)
    {
        return Attack(distiller);
    }

    return false;
}

bool AttackNagaDistillerAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh naga distiller active");
}

bool InterruptDistillerChannelAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find distiller channeling on Kalithresh using proven pattern
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NAGA_DISTILLER)
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_WARLORDS_RAGE_DISTILLER))
            {
                // Interrupt the channel
                Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
                if (spellIdsValue)
                {
                    std::list<uint32> spellIds = spellIdsValue->Get();
                    for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
                    {
                        uint32 spellId = *it;
                        if (botAI->CanCastSpell(spellId, unit, false))
                        {
                            return botAI->CastSpell(spellId, unit);
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool InterruptDistillerChannelAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh channeling rage");
}