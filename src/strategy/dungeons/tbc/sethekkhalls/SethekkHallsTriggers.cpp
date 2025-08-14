#include "SethekkHallsTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "AttackersValue.h"

bool CharmingTotemSpawnedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Don't trigger if we're charmed ourselves
    if (bot->IsCharmed())
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    bool hasTotem = false;
    bool hasCharmedAlly = false;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // Check for totem directly - don't use IsValidTarget as it might fail with charmed allies around
        if (unit->GetEntry() == NPC_CHARMING_TOTEM)
        {
            hasTotem = true;
        }

        // Also check if any group member is charmed (indicates totem is active)
        if (unit->IsPlayer() && bot->IsInSameGroupWith(unit->ToPlayer()) && unit->IsCharmed())
        {
            hasCharmedAlly = true;
        }
    }
    
    // Trigger if totem exists OR if an ally is charmed (totem might be slightly out of range)
    return hasTotem || hasCharmedAlly;
}

bool TimeLostControllerCastingTotemTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 30.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 30.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(32764))
            {
                return true;
            }
        }
    }
    return false;
}

bool IkissBlinkCastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
    if (!targetValue)
        return false;

    ObjectGuid targetGuid = targetValue->Get();
    if (!targetGuid)
        return false;

    Unit* boss = botAI->GetUnit(targetGuid);
    if (!boss || !boss->IsAlive() || boss->GetEntry() != NPC_TALON_KING_IKISS)
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BLINK_N);
}

bool IkissArcaneExplosionCastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Pattern from ForgeOfSoulsTriggers.cpp:8 and HallsOfLightningTriggers.cpp
    // Using AI_VALUE2 to find boss directly
    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss) {
        // Try alternative method to find boss
        AiObjectContext* context = botAI->GetAiObjectContext();
        if (context) {
            Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
            if (targetValue) {
                ObjectGuid targetGuid = targetValue->Get();
                if (targetGuid) {
                    Unit* target = botAI->GetUnit(targetGuid);
                    if (target && target->GetEntry() == NPC_TALON_KING_IKISS) {
                        boss = target;
                    }
                }
            }
        }
        
        if (!boss)
            return false;
    }

    // Check if boss has Arcane Bubble (preparing to explode)
    bool hasArcBubble = boss->HasAura(9438); // SPELL_ARCANE_BUBBLE from boss_talon_king_ikiss.cpp:39
    
    // Also check for the actual explosion spell
    bool hasExplosion = boss->HasAura(38197) || boss->HasAura(38198); // Normal and Heroic versions
    bool isCasting = boss->HasUnitState(UNIT_STATE_CASTING);
    
    
    return hasArcBubble || hasExplosion;
}

bool IkissArcaneExplosionEndedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
    if (!targetValue)
        return false;

    ObjectGuid targetGuid = targetValue->Get();
    if (!targetGuid)
        return false;

    Unit* boss = botAI->GetUnit(targetGuid);
    if (!boss || !boss->IsAlive() || boss->GetEntry() != NPC_TALON_KING_IKISS)
        return false;

    // Per-bot bubble state tracking to ensure all bots detect bubble end independently
    static std::map<ObjectGuid, bool> hadBubbleMap;
    ObjectGuid botGuid = bot->GetGUID();
    
    bool hasBubble = boss->HasAura(9438); // SPELL_ARCANE_BUBBLE
    bool hadBubble = hadBubbleMap[botGuid];
    
    
    if (hadBubble && !hasBubble)
    {
        hadBubbleMap[botGuid] = false;
        return true;
    }
    
    hadBubbleMap[botGuid] = hasBubble;
    return false;
}

bool SethekkSpiritNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find any Sethekk Spirit within dangerous range (20 yards)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 20.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 20.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // NPC ID 18703 is Sethekk Spirit (ghost that spawns from dead Sethekk Prophets)
        if (unit->GetEntry() == 18703)
            return true;
    }
    
    return false;
}

bool BroodOfAnzuNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // PROVEN PATTERN: Exact copy from CharmingTotemSpawnedTrigger
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_BROOD_OF_ANZU)
            return true;
    }
    
    return false;
}

