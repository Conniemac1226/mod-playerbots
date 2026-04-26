#include "SethekkHallsTriggers.h"
#include "SethekkHallsActions.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"
#include <cmath>

namespace
{
bool IsSythElemental(uint32 entry)
{
    return entry == NPC_SYTH_FIRE_ELEMENTAL ||
           entry == NPC_SYTH_FROST_ELEMENTAL ||
           entry == NPC_SYTH_ARCANE_ELEMENTAL ||
           entry == NPC_SYTH_SHADOW_ELEMENTAL;
}

bool IsGroupReadyForAdvancePull(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !bot->GetMap() || !bot->GetMap()->IsDungeon())
        return false;

    if (!bot->GetGroup() || !bot->IsAlive() || bot->IsInCombat() || !botAI->IsMainTank(bot))
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    if (context->GetValue<uint8>("attacker count")->Get() != 0)
        return false;

    if (Unit* currentTarget = context->GetValue<Unit*>("current target")->Get())
    {
        if (currentTarget->IsAlive() && currentTarget->IsInWorld() && currentTarget->GetMapId() == bot->GetMapId())
            return false;
    }

    if (bot->HealthBelowPct(AUTO_PULL_TANK_HP_PCT))
        return false;

    bool foundHealer = false;
    bool healerReady = false;

    Group* group = bot->GetGroup();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (!member->IsAlive() || member->IsBeingTeleported() || member->IsInCombat())
            return false;

        if (bot->GetMapId() != member->GetMapId() || bot->GetDistance(member) > AUTO_PULL_GROUP_RANGE)
            return false;

        if (member->HealthBelowPct(AUTO_PULL_MEMBER_HP_PCT))
            return false;

        if (!foundHealer && botAI->IsHeal(member))
        {
            foundHealer = true;
            healerReady = member->getPowerType() != POWER_MANA ||
                member->GetPowerPct(POWER_MANA) >= AUTO_PULL_HEALER_MANA_PCT;
        }
    }

    return !foundHealer || healerReady;
}

bool HasAutoPullCandidate(PlayerbotAI* botAI, Player* bot)
{
    bool hasCandidate = false;

    TbcDungeon::ForEachNearbyNpc(botAI, bot, AUTO_PULL_SEARCH_RANGE, [&](Unit* unit)
    {
        if (hasCandidate || !unit || !unit->IsAlive() || unit->IsInCombat())
            return;

        if (!AttackersValue::IsPossibleTarget(unit, bot))
            return;

        if (!bot->IsWithinLOSInMap(unit))
            return;

        if (!bot->HasInArc(static_cast<float>(M_PI) * 0.75f, unit))
            return;

        hasCandidate = true;
    });

    return hasCandidate;
}
}

bool CharmingTotemSpawnedTrigger::IsActive()
{
    // ICC Pattern (RaidIccTriggers.cpp:301-325): Simple existence check for spawned adds
    // Skull marking system handles targeting - trigger just detects presence
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_CHARMING_TOTEM && unit->IsAlive())
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

    bool castingTotem = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_MEDIUM, [&](Unit* unit)
    {
        if (castingTotem)
            return;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat() &&
            unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
        {
            castingTotem = true;
        }
    });

    return castingTotem;
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

    bool spiritNearby = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_SMALL, [&](Unit* unit)
    {
        if (spiritNearby || unit->GetEntry() != NPC_SETHEKK_SPIRIT || !unit->IsAlive())
            return;

        if (unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID())
            spiritNearby = true;
    });

    return spiritNearby;
}

bool SythNoElementalsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive())
        return false;

    Unit* syth = nullptr;
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (context)
        syth = context->GetValue<Unit*>("find target", "darkweaver syth")->Get();

    if (!syth)
    {
        if (context)
        {
            if (Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target"))
            {
                ObjectGuid targetGuid = targetValue->Get();
                if (targetGuid)
                {
                    Unit* target = botAI->GetUnit(targetGuid);
                    if (target && target->GetEntry() == NPC_DARKWEAVER_SYTH)
                        syth = target;
                }
            }
        }
    }

    if (!syth || !syth->IsAlive() || !syth->IsInCombat())
        return false;

    bool hasElementals = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (hasElementals || !unit->IsAlive())
            return;

        if (IsSythElemental(unit->GetEntry()))
            hasElementals = true;
    });

    return !hasElementals;
}

bool BroodOfAnzuNearbyTrigger::IsActive()
{
    if (botAI->IsHeal(bot)) { return false; }

    // ICC Pattern (RaidIccTriggers.cpp:301-312): No IsInCombat check for spawned adds
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_BROOD_OF_ANZU)
                return true;
        }
    }
    return false;
}

bool SethekkTankAdvanceReadyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!IsGroupReadyForAdvancePull(botAI, bot))
        return false;

    return HasAutoPullCandidate(botAI, bot);
}
