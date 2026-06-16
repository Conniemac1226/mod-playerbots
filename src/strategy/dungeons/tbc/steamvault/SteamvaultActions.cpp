#include "SteamvaultActions.h"
#include "SteamvaultTriggers.h"
#include "Group.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Playerbots.h"

// Use NPC and spell IDs from SteamvaultTriggers.h

namespace
{
Unit* GetSpellOriginalTarget(Unit* caster, uint32 spellId)
{
    if (!caster)
        return nullptr;

    Spell* spell = caster->FindCurrentSpellBySpellId(spellId);
    return spell ? spell->GetOriginalTarget() : nullptr;
}
}

// Hydromancer Thespia Actions
bool AvoidLightningCloudAction::Execute(Event event)
{
    if (!botAI)
        return false;

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
        Unit* target = GetSpellOriginalTarget(boss, SPELL_LIGHTNING_CLOUD);
        float distance = target ? bot->GetDistance(target) : bot->GetDistance(boss);

        if (distance < 25.0f) // Lightning Cloud has large AoE
        {
            // EMERGENCY: Move to safe distance
            float angle = target && target != bot ? bot->GetAngle(target) + M_PI : bot->GetAngle(boss) + M_PI;
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

    if (!boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_CLOUD))
        return false;

    Unit* target = GetSpellOriginalTarget(boss, SPELL_LIGHTNING_CLOUD);
    return bot->GetDistance(target ? target : boss) < 25.0f;
}

bool DispelLungBurstAction::Execute(Event event)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* dispelTarget = nullptr;
    float lowestHealth = 100.0f;

    auto const considerTarget = [&](Unit* target)
    {
        if (!target || !target->IsAlive() || !target->HasAura(SPELL_LUNG_BURST))
            return;

        float healthPct = target->GetHealthPct();
        if (healthPct < lowestHealth)
        {
            dispelTarget = target;
            lowestHealth = healthPct;
        }
    };

    considerTarget(bot);

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* memberRef = group->GetFirstMember(); memberRef != nullptr; memberRef = memberRef->next())
        {
            considerTarget(memberRef->GetSource());
        }
    }

    if (!dispelTarget)
        return false;

    Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel");
    if (spellIdsValue)
    {
        std::list<uint32> spellIds = spellIdsValue->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, dispelTarget, false))
            {
                return botAI->CastSpell(spellId, dispelTarget);
            }
        }
    }
    return false;
}

bool DispelLungBurstAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* lungBurstValue = botAI->GetAiObjectContext()->GetValue<bool>("thespia lung burst");
    if (!lungBurstValue)
        return false;
    
    return lungBurstValue->Get();
}

bool AttackWaterElementalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL)
        return false;

    // Find water elementals using the WotLK-style target scan
    const GuidVector npcs = AI_VALUE(GuidVector, "possible targets no los");
    Unit* elemental = nullptr;
    float closestDistance = 50.0f;

    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL)
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL)
        return false;

    return HasAttackableThespiaWaterElemental(botAI, bot);
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* shrinkRayValue = botAI->GetAiObjectContext()->GetValue<bool>("steamrigger shrink ray");
    if (!shrinkRayValue)
        return false;
    
    return shrinkRayValue->Get();
}

bool AvoidSawBladeAction::Execute(Event event)
{
    if (!botAI)
        return false;

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
        Unit* target = GetSpellOriginalTarget(boss, SPELL_SAW_BLADE);
        if (target)
        {
            if (target != bot)
                return false;
        }
        else if (boss->GetTarget() != bot->GetGUID() && boss->GetVictim() != bot)
        {
            return false;
        }

        // EMERGENCY: Move perpendicular to avoid the blade path
        float angle = target && target != bot ? bot->GetAngle(target) : bot->GetAngle(boss);
        float newAngle = angle + (frand(0, 1) > 0.5f ? M_PI / 2 : -M_PI / 2); // Random side

        float moveX = bot->GetPositionX() + cos(newAngle) * 15.0f;
        float moveY = bot->GetPositionY() + sin(newAngle) * 15.0f;
        float moveZ = bot->GetPositionZ();

        return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool AvoidSawBladeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* sawBladeValue = botAI->GetAiObjectContext()->GetValue<bool>("steamrigger saw blade");
    if (!sawBladeValue)
        return false;

    if (!sawBladeValue->Get())
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MEKGINEER_STEAMRIGGER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    Unit* target = GetSpellOriginalTarget(boss, SPELL_SAW_BLADE);
    if (target)
        return target == bot;

    return boss->GetTarget() == bot->GetGUID() || boss->GetVictim() == bot;
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("steamrigger electrified net");
    if (!boolValue)
        return false;
    
    return boolValue->Get();
}

bool AttackSteamriggerMechanicAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_STEAMRIGGER_MECHANIC)
        return false;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    Unit* mechanic = nullptr;
    float closestDistance = 50.0f;

    for (ObjectGuid const& npc : targets)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() != NPC_STEAMRIGGER_MECHANIC)
            continue;

        float distance = bot->GetDistance(unit);
        if (distance < closestDistance)
        {
            mechanic = unit;
            closestDistance = distance;
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_STEAMRIGGER_MECHANIC)
        return false;

    return HasAttackableSteamriggerMechanic(botAI, bot);
}

// Warlord Kalithresh Actions
bool StopCastingSpellReflectionAction::Execute(Event event)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "warlord kalithresh");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Spell Reflection - boss_warlord_kalithresh.cpp:79-82
    // Boss casts SPELL_SPELL_REFLECTION every 20-36 seconds via DoCastSelf.
    // Stop boss-targeted harmful spellcasts while the aura is up.
    if (boss->HasAura(SPELL_SPELL_REFLECTION))
    {
        Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!currentSpell)
            currentSpell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

        Unit* target = currentSpell ? currentSpell->GetOriginalTarget() : nullptr;
        if (currentSpell && target == boss && currentSpell->m_spellInfo && !currentSpell->m_spellInfo->IsPositive())
        {
            bot->InterruptNonMeleeSpells(true);
            botAI->GetAiObjectContext()->GetValue<bool>("spell reflection active")->Set(true);
            return true;
        }
    }

    return false;
}

bool StopCastingSpellReflectionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "warlord kalithresh");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !boss->HasAura(SPELL_SPELL_REFLECTION))
        return false;

    Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("kalithresh spell reflection");
    if (!boolValue)
        return false;
    
    if (!boolValue->Get())
        return false;

    if (!bot->IsNonMeleeSpellCast(false))
        return false;

    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell)
        currentSpell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

    Unit* target = currentSpell ? currentSpell->GetOriginalTarget() : nullptr;
    return currentSpell && target == boss && currentSpell->m_spellInfo && !currentSpell->m_spellInfo->IsPositive();
}

bool HealImpaleTargetAction::Execute(Event event)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Prioritize healing impaled targets
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* impaleTarget = nullptr;
    float lowestHealth = 100.0f;

    if (bot->HasAura(SPELL_IMPALE))
    {
        impaleTarget = bot;
        lowestHealth = bot->GetHealthPct();
    }

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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || !botAI->IsHeal(bot))
        return false;

    Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("kalithresh impale");
    if (!boolValue)
        return false;
    
    return boolValue->Get();
}

bool AttackNagaDistillerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_NAGA_DISTILLER)
        return false;

    // Find active Naga Distillers using the WotLK-style target scan
    const GuidVector npcs = AI_VALUE(GuidVector, "possible targets no los");
    Unit* distiller = nullptr;
    float closestDistance = 100.0f;

    for (ObjectGuid const& npc : npcs)
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_NAGA_DISTILLER)
        return false;

    return HasAttackableKalithreshDistiller(botAI, bot);
}

bool InterruptDistillerChannelAction::Execute(Event event)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find distiller channeling on Kalithresh using the WotLK-style target scan
    const GuidVector npcs = AI_VALUE(GuidVector, "possible targets no los");

    for (ObjectGuid const& npc : npcs)
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
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("kalithresh channeling rage");
    if (!boolValue)
        return false;
    
    return boolValue->Get();
}

bool ResumeAttackAfterSpellReflectionAction::Execute(Event event)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "warlord kalithresh");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Clear the spell reflection flag when boss no longer has the buff
    if (!boss->HasAura(SPELL_SPELL_REFLECTION))
    {
        botAI->GetAiObjectContext()->GetValue<bool>("spell reflection active")->Set(false);
        
        // Resume normal combat behavior - target the boss again if no current target
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (!currentTarget || !currentTarget->IsAlive())
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(boss);
        }
        
        return true;
    }

    return false;
}

bool ResumeAttackAfterSpellReflectionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("kalithresh spell reflection ended");
    if (!boolValue)
        return false;
    
    return boolValue->Get();
}
