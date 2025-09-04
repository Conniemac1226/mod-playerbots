#include "UnderbogActions.h"
#include "Playerbots.h"
#include "Value.h"

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
bool AttackWindcallerClawAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* bear = bot->FindNearestCreature(NPC_WINDCALLER_CLAW, 100.0f);
    if (bear && bear->IsAlive() && bear->IsInCombat())
    {
        // STRATEGY: Pet must die first - it's the primary melee threat
        // Boss stays ranged while pet attacks group members
        return Attack(bear);
    }

    return false;
}

bool AttackWindcallerClawAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* bear = bot->FindNearestCreature(NPC_WINDCALLER_CLAW, 100.0f);
    if (!bear || !bear->IsAlive() || !bear->IsInCombat())
        return false;

    // PRIORITY: Always prioritize pet over boss when pet is alive
    // The bear is the immediate melee threat while boss shoots from range
    // Only exception: if bot is already attacking the bear, continue
    Unit* currentTarget = bot->GetTarget() ? botAI->GetUnit(bot->GetTarget()) : nullptr;
    if (currentTarget && currentTarget->GetEntry() == NPC_WINDCALLER_CLAW)
        return false; // Already attacking pet, no need to change target

    return true;
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

// The Black Stalker - Position for levitate mechanic
bool BlackStalkerLevitateAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Levitate pulls players - boss_the_black_stalker.cpp:25-35
    if (bot->HasAura(UB_SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION))
    {
        // Try to position near ground to minimize fall damage
        // Move towards a wall or lower ground if possible
        float angle = bot->GetOrientation();
        float x = bot->GetPositionX() + cos(angle) * 5.0f;
        float y = bot->GetPositionY() + sin(angle) * 5.0f;
        float z = bot->GetPositionZ();
        
        // Try to move to lower ground
        bot->UpdateAllowedPositionZ(x, y, z);
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool BlackStalkerLevitateAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(UB_SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION);
}

// Attack Spore Striders
bool AttackSporeStriderAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* target = nullptr;
    Group* group = bot->GetGroup();
    
    // Priority 1: Tank's target if it's a spore strider
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && botAI->IsTank(member))
            {
                Unit* tankTarget = botAI->GetUnit(member->GetTarget());
                if (tankTarget && tankTarget->GetEntry() == NPC_SPORE_STRIDER && tankTarget->IsAlive())
                {
                    target = tankTarget;
                    break;
                }
            }
        }
    }

    // Priority 2: Find the closest spore strider to any group member
    if (!target && group)
    {
        float closestDistance = 50.0f;
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && member->IsAlive())
            {
                Unit* nearbyStrider = member->FindNearestCreature(NPC_SPORE_STRIDER, 30.0f, true);
                if (nearbyStrider)
                {
                    float distance = bot->GetDistance(nearbyStrider);
                    if (distance < closestDistance)
                    {
                        target = nearbyStrider;
                        closestDistance = distance;
                    }
                }
            }
        }
    }

    // Priority 3: Fallback to nearest strider to bot
    if (!target)
    {
        target = bot->FindNearestCreature(NPC_SPORE_STRIDER, 50.0f, true);
    }

    if (target && target->IsAlive())
    {
        return Attack(target);
    }

    return false;
}

bool AttackSporeStriderAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Allow healers to help with adds when no healing is immediately needed
    if (botAI->IsHeal(bot))
    {
        // Only help with adds if no one needs healing
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (member && member->IsAlive() && member->GetHealthPct() < 80.0f)
                    return false; // Someone needs healing, focus on that
            }
        }
    }

    Unit* strider = bot->FindNearestCreature(NPC_SPORE_STRIDER, 50.0f, true);
    return strider && strider->IsAlive();
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

bool BlackStalkerSpreadOutAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Don't move healers during active healing
    if (botAI->IsHeal(bot))
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (member && member->IsAlive() && member->GetHealthPct() < 70.0f)
                    return false; // Someone needs healing urgently
            }
        }
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && bot->GetDistance(member) < 12.0f)
        {
            float angle = bot->GetAngle(member) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool BlackStalkerSpreadOutAction::isUseful()
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

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member != bot && bot->GetDistance(member) < 12.0f)
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

    // Only spread if Chain Lightning is being cast or was recently cast
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find closest group member within chain lightning range
    Player* closestMember = nullptr;
    float closestDistance = 15.0f; // Chain lightning jump range
    
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
        // Move away from the closest member
        float angle = bot->GetAngle(closestMember) + M_PI;
        float x = bot->GetPositionX() + cos(angle) * 18.0f;
        float y = bot->GetPositionY() + sin(angle) * 18.0f;
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool BlackStalkerSpreadChainLightningAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Don't move healers if someone is critically low
    if (botAI->IsHeal(bot))
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (member && member->IsAlive() && member->GetHealthPct() < 40.0f)
                    return false; // Someone critically low, healing priority
            }
        }
    }

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
        if (member && member != bot && member->IsAlive() && bot->GetDistance(member) < 15.0f)
        {
            return true;
        }
    }

    return false;
}