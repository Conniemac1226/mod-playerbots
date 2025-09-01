#include "SlavePensTriggers.h"
#include "Playerbots.h"
#include "SlavePensActions.h"

// Mennu the Betrayer - Totems active
bool MennuTotemActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if any totems exist and are not being attacked
    uint32 totemIds[] = { NPC_NOVA_TOTEM, NPC_HEALING_WARD, NPC_EARTHGRAB_TOTEM, NPC_STONESKIN_TOTEM };
    
    for (uint32 totemId : totemIds)
    {
        Unit* totem = bot->FindNearestCreature(totemId, 50.0f);
        if (totem && totem->IsAlive() && AI_VALUE(Unit*, "current target") != totem)
            return true;
    }

    return false;
}

// Mennu casting Lightning Bolt
bool MennuLightningBoltCastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MENNU_THE_BETRAYER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Lightning Bolt cast check - boss_mennu_the_betrayer.cpp:69
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SP_SPELL_LIGHTNING_BOLT);
}

// Nova Totem nearby
bool MennuNovaTotemNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* totem = bot->FindNearestCreature(NPC_NOVA_TOTEM, 20.0f);
    return totem && totem->IsAlive() && bot->GetDistance(totem) < 10.0f;
}

// Rokmar - Ensnaring Moss on player
bool RokmarEnsnaringMossTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Ensnaring Moss root - boss_rokmar_the_crackler.cpp:60
    return bot->HasAura(SP_SPELL_ENSNARING_MOSS);
}

// Grievous Wound needs healing
bool RokmarGrievousWoundTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Grievous Wound check - boss_rokmar_the_crackler.cpp:56
    return (bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_N) || bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_H)) && bot->GetHealthPct() < 90.0f;
}

// Water Spit at low health
bool RokmarWaterSpitTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_ROKMAR_THE_CRACKLER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Water Spit at 10% health - boss_rokmar_the_crackler.cpp:64
    return boss->GetHealthPct() < 10.0f;
}

// Quagmirran - Acid Spray casting
bool QuagmirranAcidSprayTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Acid Spray frontal check - boss_quagmirran.cpp:54
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SP_SPELL_ACID_SPRAY) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Poison Bolt Volley casting
bool QuagmirranPoisonBoltVolleyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Bolt Volley check - boss_quagmirran.cpp:58
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SP_SPELL_POISON_BOLT_VOLLEY);
}

// Tank positioning for Uppercut
bool QuagmirranUppercutTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    return boss && boss->IsAlive() && boss->IsInCombat();
}

// Cleave positioning for non-tanks
bool QuagmirranCleavePositionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Cleave frontal check - boss_quagmirran.cpp:44-47
    return (botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot)) ||
           (boss->FindCurrentSpellBySpellId(SP_SPELL_CLEAVE) && boss->HasInArc(M_PI / 3, bot) && bot->GetExactDist(boss) < 10.0f);
}