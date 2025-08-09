#include "HellfireRampartsActions.h"
#include "Playerbots.h"
#include "Group.h"

// Watchkeeper Gargolmar - Attack Hellfire Watchers at 50% health
bool AttackHellfireWatcherAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Hellfire Watchers - simplified approach
    Unit* watcher = bot->FindNearestCreature(NPC_HELLFIRE_WATCHER, 100.0f);
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
    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: At 50% health watchers join fight - boss_watchkeeper_gargolmar.cpp:60
    if (boss->GetHealthPct() > 50.0f)
        return false;

    Unit* watcher = bot->FindNearestCreature(NPC_HELLFIRE_WATCHER, 100.0f);
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

    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
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
                // Move to ranged position
                // Move to ranged position
                float angle = bot->GetAngle(boss) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 20.0f;
                float y = bot->GetPositionY() + sin(angle) * 20.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Retaliation at 20% health - boss_watchkeeper_gargolmar.cpp:70
    return boss->HasAura(SPELL_RETALIATION) && bot->GetDistance(boss) < 10.0f;
}

// Omor the Unscarred - Attack Fiendish Hounds
bool AttackFiendishHoundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Fiendish Hounds - simplified approach
    Unit* hound = bot->FindNearestCreature(NPC_FIENDISH_HOUND, 50.0f);
    if (hound && hound->IsAlive() && hound->IsInCombat())
    {
        return Attack(hound);
    }

    return false;
}

bool AttackFiendishHoundAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* hound = bot->FindNearestCreature(NPC_FIENDISH_HOUND, 50.0f);
    if (hound && hound->IsAlive() && hound->IsInCombat())
        return true;

    return false;
}

// Interrupt Omor's Shadow Bolt
bool OmorShadowBoltInterruptAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Shadow Bolt spell ID from boss_omor_the_unscarred.cpp:34
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT))
    {
        // RESEARCHED: Pattern from InterruptControllerAction in SethekkHallsActions.cpp:134-143
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

bool OmorShadowBoltInterruptAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 50.0f);
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
    if (bot->HasAura(SPELL_TREACHEROUS_AURA))
    {
        // Dispel if possible
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

bool OmorTreacherousAuraAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SPELL_TREACHEROUS_AURA);
}

// Nazan & Vazruden - Avoid Liquid Fire
bool NazanLiquidFireAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Liquid Fire patches
    Unit* fire = bot->FindNearestCreature(NPC_LIQUID_FIRE, 10.0f);
    if (fire && bot->GetDistance(fire) < 8.0f)
    {
        // Move away from fire
        float angle = bot->GetAngle(fire) + M_PI;
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool NazanLiquidFireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* fire = bot->FindNearestCreature(NPC_LIQUID_FIRE, 10.0f);
    return fire && fire->IsAlive();

    return false;
}

// Avoid Cone of Fire
bool NazanConeOfFireAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    if (!nazan || !nazan->IsAlive() || !nazan->IsInCombat())
        return false;

    // RESEARCHED: Cone of Fire from boss_vazruden_the_herald.cpp:43
    if (nazan->HasUnitState(UNIT_STATE_CASTING) && nazan->FindCurrentSpellBySpellId(SPELL_CONE_OF_FIRE))
    {
        // Move behind the dragon
        float angle = nazan->GetOrientation() + M_PI;
        float x = nazan->GetPositionX() + cos(angle) * 15.0f;
        float y = nazan->GetPositionY() + sin(angle) * 15.0f;
        float z = nazan->GetPositionZ();

        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool NazanConeOfFireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
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

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
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

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    Unit* vazruden = bot->FindNearestCreature(NPC_VAZRUDEN, 100.0f);

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

    Unit* vazruden = bot->FindNearestCreature(NPC_VAZRUDEN, 100.0f);
    if (!vazruden || !vazruden->IsAlive() || !vazruden->IsInCombat())
        return false;

    return Attack(vazruden);
}

bool AttackVazrudenAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    Unit* vazruden = bot->FindNearestCreature(NPC_VAZRUDEN, 100.0f);

    // Attack Vazruden only after Nazan is dead
    return (!nazan || !nazan->IsAlive()) && vazruden && vazruden->IsAlive();
}