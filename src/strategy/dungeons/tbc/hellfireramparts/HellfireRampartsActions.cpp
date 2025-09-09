#include "HellfireRampartsActions.h"
#include "Playerbots.h"
#include "Group.h"
#include <cfloat>

// Watchkeeper Gargolmar - Attack Hellfire Watchers at 50% health
bool AttackHellfireWatcherAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Hellfire Watchers - simplified approach
    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (watcher && watcher->IsAlive() && watcher->IsInCombat())
    {
        return Attack(watcher);
    }

    return false;
}

bool AttackHellfireWatcherAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Gargolmar is below 50% health and watchers exist
    Unit* boss = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: At 50% health watchers join fight - boss_watchkeeper_gargolmar.cpp:60
    if (boss->GetHealthPct() > 50.0f)
        return false;

    Unit* watcher = AI_VALUE2(Unit*, "find target", "hellfire watcher");
    if (watcher && watcher->IsAlive() && watcher->IsInCombat())
        return true;

    return false;
}

// Gargolmar uses Retaliation at 20% health
bool GargolmarRetaliationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Retaliation buff active - boss_watchkeeper_gargolmar.cpp:71
    if (boss->HasAura(SPELL_RETALIATION))
    {
        // Ranged classes should switch to ranged attacks
        if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE || 
            bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_WARLOCK)
        {
            float distance = bot->GetDistance(boss);
            if (distance < 10.0f)
            {
                // Move to ranged position using MoveAway pattern from WOTLK
                return MoveAway(boss, 15.0f - distance);
            }
        }
    }

    return false;
}

bool GargolmarRetaliationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Retaliation at 20% health - boss_watchkeeper_gargolmar.cpp:70
    return boss->HasAura(SPELL_RETALIATION) && bot->GetDistance(boss) < 10.0f;
}

// Handle Gargolmar's Surge (targets farthest player)
bool GargolmarSurgeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Surge targets min distance (farthest) - boss_watchkeeper_gargolmar.cpp:91
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SURGE))
    {
        // If we're ranged and far away, we might be the target - move closer
        float distance = bot->GetDistance(boss);
        if (distance > 20.0f)
        {
            // Move closer to avoid being surge target
            float angle = bot->GetAngle(boss);
            float x = bot->GetPositionX() + cos(angle) * 5.0f;
            float y = bot->GetPositionY() + sin(angle) * 5.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool GargolmarSurgeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "watchkeeper gargolmar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only useful for ranged who are too far
    return bot->GetDistance(boss) > 20.0f;
}

// Omor the Unscarred - Attack Fiendish Hounds
bool AttackFiendishHoundAction::isUseful() { return !botAI->IsHeal(bot); }
bool AttackFiendishHoundAction::Execute(Event event)
{
    Unit* hound = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_FIENDISH_HOUND)
        {
            hound = unit;
            break;
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Prevent ping-pong between multiple hounds if we're attacking one already
    if (hound && currentTarget && currentTarget->GetEntry() == NPC_FIENDISH_HOUND)
    {
        return false;
    }

    if (!hound || AI_VALUE(Unit*, "current target") == hound)
    {
        return false;
    }
    
    return Attack(hound);
}


// Interrupt Omor's Shadow Bolt
bool OmorShadowBoltInterruptAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Shadow Bolt spell ID from boss_omor_the_unscarred.cpp:34
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT))
    {
        // RESEARCHED: Pattern from InterruptControllerAction in SethekkHallsActions.cpp:134-143
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

bool OmorShadowBoltInterruptAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT);
}

// Handle Treacherous Aura
bool OmorTreacherousAuraAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Treacherous Aura from boss_omor_the_unscarred.cpp:36
    // Some cores use a different aura id on players (heroic): SPELL_TREACHEROUS_AURA_H
    if (bot->HasAura(SPELL_TREACHEROUS_AURA) || bot->HasAura(SPELL_TREACHEROUS_AURA_H))
    {
        // Emergency: ensure immediate group spread even if cast trigger was missed
        if (AI_VALUE(float, "disperse distance") < 15.0f)
        {
            SET_AI_VALUE(float, "disperse distance", 20.0f);
        }
        // Dispel if possible
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
        // Even if we cannot dispel, consider the spread action handled
        return true;
    }

    return false;
}

bool OmorTreacherousAuraAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SPELL_TREACHEROUS_AURA);
}

// Handle Omor's Demonic Shield at 21% health
bool OmorDemonicShieldAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
        
    // RESEARCHED: Demonic Shield at 21% - boss_omor_the_unscarred.cpp:56-62
    if (boss->HasAura(SPELL_DEMONIC_SHIELD))
    {
        // Stop attacking Omor during shield, focus adds if any
        const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        for (const auto& guid : targets) {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_FIENDISH_HOUND) {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(unit);
                return true;
            }
        }
        
        // NO ADDS: Don't block other actions - let bots continue normal behavior
        // Shield makes Omor immune but bots can still move, heal, buff, etc.
        return false;
    }

    return false;
}

bool OmorDemonicShieldAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasAura(SPELL_DEMONIC_SHIELD);
}


// Nazan & Vazruden - Avoid Liquid Fire
bool NazanLiquidFireAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Liquid Fire is a ground effect, not an NPC - need to check for area effect
    // Check for nearby fire patches using direct creature search
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // RESEARCHED: Liquid Fire summoned by SPELL_SUMMON_LIQUID_FIRE (31706) - HellfireRampartsActions.h:21
        if (unit->GetEntry() == NPC_LIQUID_FIRE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 8.0f)
            {
                return MoveAway(unit, 10.0f - distance);
            }
        }
    }

    return false;
}

bool NazanLiquidFireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if any liquid fire patches are near the bot
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_LIQUID_FIRE)
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }

    return false;
}

// Avoid Cone of Fire
bool NazanConeOfFireAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    if (!nazan || !nazan->IsAlive() || !nazan->IsInCombat())
        return false;

    // RESEARCHED: Cone of Fire from boss_vazruden_the_herald.cpp:43 and :210
    // Only happens when Nazan has landed (phase 2)
    if (!nazan->IsLevitating() && nazan->HasUnitState(UNIT_STATE_CASTING) && 
        nazan->FindCurrentSpellBySpellId(SPELL_CONE_OF_FIRE))
    {
        // Check if we're in front arc
        if (nazan->HasInArc(M_PI / 2, bot))
        {
            // Move behind the dragon
            float angle = nazan->GetOrientation() + M_PI;
            float x = nazan->GetPositionX() + cos(angle) * 10.0f;
            float y = nazan->GetPositionY() + sin(angle) * 10.0f;
            float z = nazan->GetPositionZ();

            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool NazanConeOfFireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    if (!nazan || !nazan->IsAlive() || !nazan->IsInCombat())
        return false;

    // Check if Nazan is casting Cone of Fire and we're in front
    if (nazan->HasUnitState(UNIT_STATE_CASTING) && nazan->FindCurrentSpellBySpellId(SPELL_CONE_OF_FIRE))
    {
        // Check if we're in front arc
        return nazan->HasInArc(M_PI / 2, bot);
    }

    return false;
}

// Attack Nazan first when both are up
bool AttackNazanFirstAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    if (!nazan || !nazan->IsAlive() || !nazan->IsInCombat())
        return false;

    // Check if Nazan has landed (not flying)
    if (!nazan->IsLevitating())
    {
        return Attack(nazan);
    }

    return false;
}

bool AttackNazanFirstAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");

    // Both must be alive and Nazan must have landed
    if (nazan && nazan->IsAlive() && !nazan->IsLevitating() &&
        vazruden && vazruden->IsAlive())
    {
        // Current target should not be Nazan
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        return currentTarget && currentTarget->GetEntry() != NPC_NAZAN;
    }

    return false;
}

// Attack Vazruden when Nazan is dead
bool AttackVazrudenAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");
    if (!vazruden || !vazruden->IsAlive() || !vazruden->IsInCombat())
        return false;

    return Attack(vazruden);
}

bool AttackVazrudenAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = AI_VALUE2(Unit*, "find target", "nazan");
    Unit* vazruden = AI_VALUE2(Unit*, "find target", "vazruden");

    // Attack Vazruden only after Nazan is dead
    return (!nazan || !nazan->IsAlive()) && vazruden && vazruden->IsAlive();
}

// Handle Nazan's Bellowing Roar (Heroic only)
bool NazanBellowingRoarAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Bellowing Roar fear in heroic - boss_vazruden_the_herald.cpp:218-225
    if (bot->HasAura(SPELL_BELLOWING_ROAR))
    {
        // Use fear ward or tremor totem if available
        if (bot->getClass() == CLASS_PRIEST)
        {
            if (botAI->CanCastSpell(6346, bot, false)) // Fear Ward
            {
                return botAI->CastSpell(6346, bot);
            }
        }
        else if (bot->getClass() == CLASS_SHAMAN)
        {
            if (botAI->CanCastSpell(8143, bot, false)) // Tremor Totem
            {
                return botAI->CastSpell(8143, bot);
            }
        }
        
        // Try to dispel fear
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

bool NazanBellowingRoarAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only in heroic
    if (bot->GetMap()->GetDifficulty() != DUNGEON_DIFFICULTY_HEROIC)
        return false;

    return bot->HasAura(SPELL_BELLOWING_ROAR);
}


// NEW HEROIC-READY SPREAD MECHANICS FOR OMOR

bool OmorTreacherySpreadAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Ulduar/ICC Pattern: Use disperse distance AI value for immediate spread
    // RESEARCHED: Same pattern used in RaidUlduarActions.cpp:2269 (Mimiron) and RaidIccActions.cpp
    SET_AI_VALUE(float, "disperse distance", 20.0f);
    
    return true;
}

bool OmorTreacherySpreadAction::isUseful()
{
    // Always useful when trigger activates - this is emergency spread
    return true;
}

bool OmorDebuffAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find the closest player with Treacherous Aura debuff
    Unit* debuffedPlayer = nullptr;
    float closestDistance = FLT_MAX;
    
    GuidVector friendlyUnits = AI_VALUE(GuidVector, "nearest friendly players");
    for (const auto& guid : friendlyUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit != bot && unit->IsAlive() && (unit->HasAura(SPELL_TREACHEROUS_AURA) || unit->HasAura(SPELL_TREACHEROUS_AURA_H)))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                debuffedPlayer = unit;
            }
        }
    }
    
    if (debuffedPlayer)
    {
        // Ensure group-wide spread is active during this window
        if (AI_VALUE(float, "disperse distance") < 15.0f)
        {
            SET_AI_VALUE(float, "disperse distance", 20.0f);
        }
        // Move away only if too close
        if (closestDistance < 15.0f)
        {
            return MoveAway(debuffedPlayer, 25.0f - closestDistance, false);
        }
        return true;
    }

    return false;
}

bool OmorDebuffAvoidanceAction::isUseful()
{
    return true; // Always useful when trigger activates
}

bool OmorClearSpreadAction::Execute(Event event)
{
    // Clear the disperse distance when cast finishes or fight ends
    if (AI_VALUE(float, "disperse distance") > 0.0f)
    {
        SET_AI_VALUE(float, "disperse distance", 0.0f);
        return true;
    }
    return false;
}

bool OmorClearSpreadAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!boss)
        return true; // Clear spread when boss is gone
    
    // Clear spread when Omor is not casting Treacherous Aura
    return !(boss->HasUnitState(UNIT_STATE_CASTING) && 
             boss->FindCurrentSpellBySpellId(SPELL_TREACHEROUS_AURA));
}
