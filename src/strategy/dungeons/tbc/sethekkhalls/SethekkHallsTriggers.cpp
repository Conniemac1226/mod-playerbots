#include "SethekkHallsTriggers.h"
#include "SethekkHallsActions.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "AttackersValue.h"

bool CharmingTotemSpawnedTrigger::IsActive()
{
    if (botAI->IsHeal(bot)) { return false; }

    if (bot->IsCharmed())
        return false;

    // WotLK pattern for spawned adds
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_CHARMING_TOTEM)
        {
            return true;
        }
    }
    return false;
}

bool TimeLostControllerCastingTotemTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_MEDIUM);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_MEDIUM);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
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

    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss) {
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

    bool hasArcBubble = boss->HasAura(SPELL_ARCANE_BUBBLE); // SPELL_ARCANE_BUBBLE from boss_talon_king_ikiss.cpp:39
    
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

    static std::map<ObjectGuid, bool> hadBubbleMap;
    ObjectGuid botGuid = bot->GetGUID();
    
    bool hasBubble = boss->HasAura(SPELL_ARCANE_BUBBLE); // SPELL_ARCANE_BUBBLE
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

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_SMALL);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_SMALL);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == 18703)
            return true;
    }
    
    return false;
}

bool BroodOfAnzuNearbyTrigger::IsActive()
{
    if (botAI->IsHeal(bot)) { return false; }

    // WotLK pattern for spawned adds
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_BROOD_OF_ANZU)
        {
            return true;
        }
    }
    return false;
}

