#include "UnderbogActions.h"
#include "Playerbots.h"

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
    if (boss->GetHealthPct() <= 20.0f && boss->HasAura(SPELL_FOUL_SPORES))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f) // Foul Spores has large radius
        {
            // Move to max range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    return boss->GetHealthPct() <= 20.0f && boss->HasAura(SPELL_FOUL_SPORES);
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
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ACID_BREATH))
    {
        // Check if we're in front arc
        if (boss->HasInArc(M_PI / 3, bot))
        {
            // Move to side/behind
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ACID_BREATH) && 
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
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_TAIL_SWEEP))
    {
        // Check if we're behind boss
        if (!boss->HasInArc(M_PI, bot))
        {
            // Move to side
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    // Only useful if we're behind the boss
    return !boss->HasInArc(M_PI, bot);
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
    return bear && bear->IsAlive() && bear->IsInCombat();
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
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_THROW_FREEZING_TRAP))
    {
        // Move away from current position (trap lands at target location)
        float angle = bot->GetAngle(boss) + (M_PI / 4);
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_THROW_FREEZING_TRAP);
}

// Dispel Hunter's Mark
bool MuselekHuntersMarkAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Hunter's Mark debuff - boss_swamplord_muselek.cpp:155
    if (bot->HasAura(SPELL_HUNTERS_MARK))
    {
        // Try to dispel the mark
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, bot, false))
            {
                return botAI->CastSpell(spellId, bot);
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

    return bot->HasAura(SPELL_HUNTERS_MARK);
}

// The Black Stalker - Position for levitate mechanic
bool BlackStalkerLevitateAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Levitate pulls players - boss_the_black_stalker.cpp:25-35
    if (bot->HasAura(SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION))
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

    return bot->HasAura(SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION);
}

// Attack Spore Striders
bool AttackSporeStriderAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* strider = bot->FindNearestCreature(NPC_SPORE_STRIDER, 50.0f);
    if (strider && strider->IsAlive() && strider->IsInCombat())
    {
        return Attack(strider);
    }

    return false;
}

bool AttackSporeStriderAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* strider = bot->FindNearestCreature(NPC_SPORE_STRIDER, 50.0f);
    return strider && strider->IsAlive() && strider->IsInCombat();
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

    // RESEARCHED: Chain Lightning cast - boss_the_black_stalker.cpp:72
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_CHAIN_LIGHTNING))
    {
        // RESEARCHED: Pattern from HellfireRampartsActions.cpp:138-145
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return botAI->CastSpell(spellId, boss);
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

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_CHAIN_LIGHTNING);
}