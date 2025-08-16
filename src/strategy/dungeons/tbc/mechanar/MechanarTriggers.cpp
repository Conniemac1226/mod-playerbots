#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"

// ========== MECHANO LORD CAPACITUS TRIGGERS ==========

bool CapacitusEngagedTrigger::IsActive()
{
    // Using proven WotLK pattern
    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool ReflectiveShieldActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->GetMap()->IsHeroic())
        return false; // Normal mode only

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasAura(SPELL_REFLECTIVE_MAGIC_SHIELD) || 
           boss->HasAura(SPELL_REFLECTIVE_DAMAGE_SHIELD);
}

bool PolarityShiftActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false; // Heroic only

    return bot->HasAura(SPELL_POSITIVE_POLARITY) || 
           bot->HasAura(SPELL_NEGATIVE_POLARITY);
}

bool NetherChargeActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for Nether Charges using proven WotLK pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_CHARGE)
            return true;
    }
    
    return false;
}

// ========== NETHERMANCER SEPETHREA TRIGGERS ==========

bool SepethreaEngagedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
        
    return true;
}

bool RagingFlamesActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        if (bot->GetDistance(flame) < 30.0f)
            return true;
    }
    
    return false;
}

bool DragonsBreathDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Dragon's Breath
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_DRAGONS_BREATH))
    {
        // Check if we're in the frontal cone (wider detection for safety)
        return boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot);
    }

    return false;
}


bool RagingFlamesTargetTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    if (botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false;
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_RAGING_FLAMES)
            return true;
    }
    
    return false;
}

// ========== PATHALEON THE CALCULATOR TRIGGERS ==========

bool PathaleonEngagedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool DominationActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we're dominated
    if (bot->HasAura(SPELL_DOMINATION))
        return true;

    // Check if any ally is dominated (need to CC them)
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        if (ally->HasAura(SPELL_DOMINATION))
            return true;
    }

    return false;
}

bool NetherWraithActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for Nether Wraiths
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_WRAITH)
            return true;
    }
    
    return false;
}

bool ArcaneTorrentDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Arcane Torrent
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_ARCANE_TORRENT))
    {
        return bot->GetDistance(boss) < 20.0f && !botAI->IsMelee(bot);
    }

    return false;
}

bool PathaleonEnragedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    return boss && boss->HasAura(SPELL_FRENZY);
}

bool ArcaneExplosionDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Arcane Explosion
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION))
    {
        return bot->GetDistance(boss) < 10.0f && !botAI->IsMelee(bot);
    }

    return false;
}

bool ManaTapActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    return bot && bot->getPowerType() == POWER_MANA && bot->HasAura(SPELL_MANA_TAP);
}