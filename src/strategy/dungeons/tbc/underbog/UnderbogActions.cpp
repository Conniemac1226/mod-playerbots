#include "UnderbogActions.h"
#include "Playerbots.h"
#include "Value.h"
#include <unordered_map>

namespace
{
    std::unordered_map<ObjectGuid, bool> blackStalkerLevitateHandledByBot;

    bool IsBlackStalkerLevitateActive(Player* bot)
    {
        return bot && (bot->HasAura(UB_SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION));
    }

    bool NeedsBlackStalkerLevitateResponse(Player* bot)
    {
        if (!bot)
            return false;

        ObjectGuid const guid = bot->GetGUID();
        if (!IsBlackStalkerLevitateActive(bot))
        {
            blackStalkerLevitateHandledByBot.erase(guid);
            return false;
        }

        return blackStalkerLevitateHandledByBot.find(guid) == blackStalkerLevitateHandledByBot.end();
    }
}

// Hungarfen - Avoid mushroom explosions at 20% health
bool HungarfenMushroomAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* mushroom = bot->FindNearestCreature(NPC_UNDERBOG_MUSHROOM, 15.0f);
    if (mushroom && mushroom->IsAlive())
    {
        float distance = bot->GetDistance(mushroom);
        if (distance < 10.0f) // Mushrooms explode in ~10 yard radius
        {
            // Move away from mushroom
            float angle = bot->GetAngle(mushroom) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool HungarfenMushroomAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HUNGARFEN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Mushrooms spawn at 20% health - boss_hungarfen.cpp:117
    if (boss->GetHealthPct() > 20.0f)
        return false;

    Unit* mushroom = bot->FindNearestCreature(NPC_UNDERBOG_MUSHROOM, 15.0f);
    return mushroom && mushroom->IsAlive() && bot->GetDistance(mushroom) < 10.0f;
}

// Avoid Foul Spores AoE
bool HungarfenFoulSporesAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HUNGARFEN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Foul Spores at 20% health - boss_hungarfen.cpp:59
    // Boss roots himself and channels for 11 seconds - evacuate immediately!
    if (boss->GetHealthPct() <= 20.0f && 
        (boss->FindCurrentSpellBySpellId(UB_SPELL_FOUL_SPORES) || boss->HasAura(UB_SPELL_FOUL_SPORES)))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 25.0f) // Foul Spores has large radius - stay far!
        {
            // EMERGENCY: Move to max range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool HungarfenFoulSporesAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HUNGARFEN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->GetHealthPct() <= 20.0f && 
           (boss->FindCurrentSpellBySpellId(UB_SPELL_FOUL_SPORES) || boss->HasAura(UB_SPELL_FOUL_SPORES)) && 
           bot->GetDistance(boss) < 25.0f;
}

// Ghazan - Avoid Acid Breath frontal cone
bool GhazanAcidBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Acid Breath frontal cone - boss_ghazan.cpp:69
    // Use immediate detection for faster response
    if (boss->FindCurrentSpellBySpellId(UB_SPELL_ACID_BREATH))
    {
        // Check if we're in front arc (60 degree cone)
        if (boss->HasInArc(M_PI / 3, bot))
        {
            // EMERGENCY: Move to side/behind
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool GhazanAcidBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->FindCurrentSpellBySpellId(UB_SPELL_ACID_BREATH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Avoid Tail Sweep behind boss
bool GhazanTailSweepAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Tail Sweep behind - boss_ghazan.cpp:77
    // Tail Sweep hits BEHIND the boss - move if we're behind!
    if (boss->FindCurrentSpellBySpellId(UB_SPELL_TAIL_SWEEP))
    {
        // Check if we're behind boss (180 degree arc behind)
        float angle_diff = fabs(bot->GetRelativeAngle(boss));
        if (angle_diff > M_PI * 2.0f / 3.0f) // Behind = more than 120 degrees from front
        {
            // EMERGENCY: Move to side/front
            float angle = boss->GetOrientation() + (M_PI / 3); // Move to 60 degrees from front
            float x = boss->GetPositionX() + cos(angle) * 8.0f;
            float y = boss->GetPositionY() + sin(angle) * 8.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool GhazanTailSweepAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only useful if we're behind the boss where tail sweep hits
    float angle_diff = fabs(bot->GetRelativeAngle(boss));
    return boss->FindCurrentSpellBySpellId(UB_SPELL_TAIL_SWEEP) && 
           (angle_diff > M_PI * 2.0f / 3.0f); // Behind = more than 120 degrees from front
}

// Swamplord Musel'ek - Attack bear pet first
bool AttackWindcallerClawAction::isUseful() { return !botAI->IsHeal(bot); }
bool AttackWindcallerClawAction::Execute(Event event)
{
    Unit* target = AI_VALUE2(Unit*, "find target", "windcaller claw");
    if (!target || !target->IsAlive() || !target->IsInCombat() ||
        AI_VALUE(Unit*, "current target") == target)
    {
        return false;
    }
    return Attack(target);
}


// Avoid Freezing Trap
bool MuselekFreezingTrapAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SWAMPLORD_MUSELEK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Freezing trap throw - boss_swamplord_muselek.cpp:138
    // Trap is thrown at player's current location - MOVE IMMEDIATELY!
    if (boss->FindCurrentSpellBySpellId(UB_SPELL_THROW_FREEZING_TRAP))
    {
        // Check if we're the target
        if (boss->GetTarget() == bot->GetGUID() || boss->GetVictim() == bot)
        {
            // EMERGENCY: Move away from current position (trap lands here!)
            float angle = frand(0, 2 * M_PI); // Random direction for better spread
            float x = bot->GetPositionX() + cos(angle) * 12.0f;
            float y = bot->GetPositionY() + sin(angle) * 12.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool MuselekFreezingTrapAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SWAMPLORD_MUSELEK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_THROW_FREEZING_TRAP);
}

// Dispel Hunter's Mark
bool MuselekHuntersMarkAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Hunter's Mark debuff - boss_swamplord_muselek.cpp:155
    if (bot->HasAura(UB_SPELL_HUNTERS_MARK))
    {
        // Try to dispel the mark - SAFE PATTERN
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

bool MuselekHuntersMarkAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(UB_SPELL_HUNTERS_MARK);
}

// The Black Stalker - One-shot response for the levitate / suspension chain
bool BlackStalkerLevitateAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Levitate pulls players - boss_the_black_stalker.cpp:25-35
    if (!NeedsBlackStalkerLevitateResponse(bot))
        return false;

    // Move once, then stay put until the aura chain ends.
    float angle = bot->GetOrientation();
    float x = bot->GetPositionX() + cos(angle) * 5.0f;
    float y = bot->GetPositionY() + sin(angle) * 5.0f;
    float z = bot->GetPositionZ();

    // Try to move to lower ground
    bot->UpdateAllowedPositionZ(x, y, z);

    if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
               MovementPriority::MOVEMENT_FORCED))
    {
        blackStalkerLevitateHandledByBot[bot->GetGUID()] = true;
        return true;
    }

    return false;
}

bool BlackStalkerLevitateAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return NeedsBlackStalkerLevitateResponse(bot);
}

// Attack Spore Striders
bool AttackSporeStriderAction::isUseful() { return !botAI->IsHeal(bot); }
bool AttackSporeStriderAction::Execute(Event event)
{
    Unit* strider = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SPORE_STRIDER)
        {
            strider = unit;
            break;
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Prevent ping-pong between multiple striders if we're attacking one already
    if (strider && currentTarget && currentTarget->GetEntry() == NPC_SPORE_STRIDER)
    {
        return false;
    }

    if (!strider || AI_VALUE(Unit*, "current target") == strider)
    {
        return false;
    }
    
    return Attack(strider);
}


// Interrupt Chain Lightning
bool BlackStalkerChainLightningAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING))
    {
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
            }
        }
    }

    return false;
}

bool BlackStalkerChainLightningAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING);
}

bool BlackStalkerStaticChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float safeDistance = 18.0f;

    if (bot->HasAura(UB_SPELL_STATIC_CHARGE))
    {
        Player* nearestMember = nullptr;
        float nearestDistance = 1000.0f;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && member != bot && member->IsAlive())
            {
                float distance = bot->GetDistance(member);
                if (distance < nearestDistance)
                {
                    nearestMember = member;
                    nearestDistance = distance;
                }
            }
        }

        if (!nearestMember || nearestDistance >= safeDistance)
            return false;

        return MoveAway(nearestMember, safeDistance - nearestDistance);
    }

    Player* chargedMember = nullptr;
    float chargedDistance = 1000.0f;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && member->IsAlive() &&
            member->HasAura(UB_SPELL_STATIC_CHARGE))
        {
            float distance = bot->GetDistance(member);
            if (distance < chargedDistance)
            {
                chargedMember = member;
                chargedDistance = distance;
            }
        }
    }

    if (!chargedMember || chargedDistance >= safeDistance)
        return false;

    return MoveAway(chargedMember, safeDistance - chargedDistance);
}

bool BlackStalkerStaticChargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float safeDistance = 18.0f;

    if (bot->HasAura(UB_SPELL_STATIC_CHARGE))
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && member != bot && member->IsAlive() &&
                bot->GetDistance(member) < safeDistance)
            {
                return true;
            }
        }

        return false;
    }

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && member->IsAlive() &&
            member->HasAura(UB_SPELL_STATIC_CHARGE) &&
            bot->GetDistance(member) < safeDistance)
        {
            return true;
        }
    }

    return false;
}

// Chain Lightning specific spread action - triggers immediately on cast
bool BlackStalkerSpreadChainLightningAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only spread while Chain Lightning is being cast
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find closest group member within chain lightning range
    Player* closestMember = nullptr;
    float closestDistance = 18.0f;
    
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && member->IsAlive())
        {
            float distance = bot->GetDistance(member);
            if (distance < closestDistance)
            {
                closestMember = member;
                closestDistance = distance;
            }
        }
    }

    if (closestMember)
    {
        float currentDistance = bot->GetDistance(closestMember);
        return MoveAway(closestMember, 18.0f - currentDistance);
    }

    return false;
}

bool BlackStalkerSpreadChainLightningAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only useful if Chain Lightning is being cast
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Check if we're too close to anyone
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && member->IsAlive() &&
            bot->GetDistance(member) < 18.0f)
        {
            return true;
        }
    }

    return false;
}
