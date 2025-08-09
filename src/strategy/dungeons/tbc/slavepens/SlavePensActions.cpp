#include "SlavePensActions.h"
#include "Playerbots.h"

// Mennu the Betrayer - Attack totems with priority
bool AttackMennuTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Priority order: Nova Totem > Healing Ward > Earthgrab > Stoneskin
    uint32 totemPriority[] = { NPC_NOVA_TOTEM, NPC_HEALING_WARD, NPC_EARTHGRAB_TOTEM, NPC_STONESKIN_TOTEM };
    
    for (uint32 totemId : totemPriority)
    {
        Unit* totem = bot->FindNearestCreature(totemId, 50.0f);
        if (totem && totem->IsAlive())
        {
            return Attack(totem);
        }
    }

    return false;
}

bool AttackMennuTotemAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if any totems exist
    uint32 totemIds[] = { NPC_NOVA_TOTEM, NPC_HEALING_WARD, NPC_EARTHGRAB_TOTEM, NPC_STONESKIN_TOTEM };
    
    for (uint32 totemId : totemIds)
    {
        Unit* totem = bot->FindNearestCreature(totemId, 50.0f);
        if (totem && totem->IsAlive())
            return true;
    }

    return false;
}

// Interrupt Mennu's Lightning Bolt
bool MennuLightningBoltInterruptAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MENNU_THE_BETRAYER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Lightning Bolt cast - boss_mennu_the_betrayer.cpp:69
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_BOLT))
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

bool MennuLightningBoltInterruptAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MENNU_THE_BETRAYER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_BOLT);
}

// Move away from Nova Totem explosion
bool MennuNovaTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* totem = bot->FindNearestCreature(NPC_NOVA_TOTEM, 20.0f);
    if (totem && totem->IsAlive())
    {
        float distance = bot->GetDistance(totem);
        if (distance < 10.0f) // Nova Totem has ~10 yard explosion radius
        {
            // Move away from totem
            float angle = bot->GetAngle(totem) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool MennuNovaTotemAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* totem = bot->FindNearestCreature(NPC_NOVA_TOTEM, 20.0f);
    return totem && totem->IsAlive() && bot->GetDistance(totem) < 10.0f;
}

// Rokmar - Dispel Ensnaring Moss
bool RokmarEnsnaringMossAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Ensnaring Moss - boss_rokmar_the_crackler.cpp:60
    if (bot->HasAura(SPELL_ENSNARING_MOSS))
    {
        // Try to dispel the root effect
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

bool RokmarEnsnaringMossAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SPELL_ENSNARING_MOSS);
}

// Heal Grievous Wound
bool RokmarGrievousWoundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Grievous Wound requires healing above 90% - boss_rokmar_the_crackler.cpp:56
    if (bot->HasAura(SPELL_GRIEVOUS_WOUND_N) || bot->HasAura(SPELL_GRIEVOUS_WOUND_H))
    {
        if (bot->GetHealthPct() < 90.0f)
        {
            // Use healing abilities if available
            std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal")->Get();
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

bool RokmarGrievousWoundAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return (bot->HasAura(SPELL_GRIEVOUS_WOUND_N) || bot->HasAura(SPELL_GRIEVOUS_WOUND_H)) && bot->GetHealthPct() < 90.0f;
}

// Move away from Water Spit
bool RokmarWaterSpitAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_ROKMAR_THE_CRACKLER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Water Spit at 10% health - boss_rokmar_the_crackler.cpp:64
    if (boss->GetHealthPct() < 10.0f && boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // Spread out to avoid chain damage
        float angle = bot->GetAngle(boss) + (M_PI / 4);
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool RokmarWaterSpitAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_ROKMAR_THE_CRACKLER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->GetHealthPct() < 10.0f;
}

// Quagmirran - Avoid Acid Spray cone
bool QuagmirranAcidSprayAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Acid Spray frontal cone - boss_quagmirran.cpp:54
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ACID_SPRAY))
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

bool QuagmirranAcidSprayAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ACID_SPRAY) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Interrupt Poison Bolt Volley
bool QuagmirranPoisonBoltVolleyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Bolt Volley AoE - boss_quagmirran.cpp:58
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_BOLT_VOLLEY))
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

bool QuagmirranPoisonBoltVolleyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_BOLT_VOLLEY);
}

// Tank positioning for Uppercut
bool QuagmirranUppercutAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only tanks need to worry about positioning
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Uppercut knockback - boss_quagmirran.cpp:50
    // Position boss away from edges to avoid being knocked off
    // This is a general tank positioning reminder
    float distance = bot->GetDistance(boss);
    if (distance > 5.0f && botAI->IsTank(bot))
    {
        // Move closer to boss to maintain threat
        Position pos = boss->GetPosition();
        float angle = bot->GetAngle(boss);
        float newDist = 3.0f;
        pos.m_positionX += cos(angle) * newDist;
        pos.m_positionY += sin(angle) * newDist;
        
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ, 
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool QuagmirranUppercutAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    return boss && boss->IsAlive() && boss->IsInCombat();
}