#include "SlavePensActions.h"
#include "Playerbots.h"
#include "Value.h"

// Mennu the Betrayer - Attack totems with priority
bool AttackMennuTotemAction::Execute(Event event)
{
    // RESEARCHED: Totems spawn pattern - boss_mennu_the_betrayer.cpp:71-83
    // Priority: Nova (explosion) > Healing (60% hp) > Earthgrab (root) > Stoneskin (armor)
    uint32 totemPriority[] = { NPC_NOVA_TOTEM, NPC_HEALING_WARD, NPC_EARTHGRAB_TOTEM, NPC_STONESKIN_TOTEM };
    
    Unit* priorityTotem = nullptr;
    
    // WotLK pattern with priority - find highest priority totem
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (uint32 totemId : totemPriority)
    {
        for (auto& target : targets)
        {
            Unit* unit = botAI->GetUnit(target);
            if (unit && unit->IsInCombat() && unit->GetEntry() == totemId)
            {
                priorityTotem = unit;
                break; // Found highest priority, stop searching
            }
        }
        if (priorityTotem) break; // Found totem, stop checking lower priorities
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Prevent ping-pong between totems if attacking one already (unless higher priority found)
    if (priorityTotem && currentTarget)
    {
        // Check if current target is a totem
        bool currentIsTotem = false;
        for (uint32 totemId : totemPriority)
        {
            if (currentTarget->GetEntry() == totemId)
            {
                currentIsTotem = true;
                break;
            }
        }
        
        // If attacking a totem and found same/lower priority, don't switch
        if (currentIsTotem && priorityTotem == currentTarget)
        {
            return false;
        }
    }

    if (!priorityTotem || AI_VALUE(Unit*, "current target") == priorityTotem)
    {
        return false;
    }
    
    return Attack(priorityTotem);
}

bool AttackMennuTotemAction::isUseful()
{
    return !botAI->IsHeal(bot);
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

    // RESEARCHED: Lightning Bolt every 7-10s - boss_mennu_the_betrayer.cpp:67-70
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_LIGHTNING_BOLT))
    {
        // Interrupt immediately to prevent damage
        if (bot->IsWithinMeleeRange(boss))
        {
            botAI->InterruptSpell();
            return true;
        }
        
        // Ranged interrupts - SAFE PATTERN
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

bool MennuLightningBoltInterruptAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MENNU_THE_BETRAYER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SP_SPELL_LIGHTNING_BOLT);
}

// Move away from Nova Totem explosion
bool MennuNovaTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Nova Totem explodes after delay - boss_mennu_the_betrayer.cpp:73
    // Move away IMMEDIATELY when totem spawns
    std::list<Creature*> totemList;
    bot->GetCreatureListWithEntryInGrid(totemList, NPC_NOVA_TOTEM, 25.0f);
    
    for (Creature* totem : totemList)
    {
        if (totem && totem->IsAlive())
        {
            float distance = bot->GetExactDist(totem);
            float safeDistance = 15.0f; // Nova explosion ~10 yards + safety
            
            if (distance < safeDistance)
            {
                // Emergency movement away from Nova Totem
                float angle = totem->GetAngle(bot) + M_PI;
                float moveDistance = safeDistance - distance + 3.0f;
                float x = totem->GetPositionX() + cos(angle) * (distance + moveDistance);
                float y = totem->GetPositionY() + sin(angle) * (distance + moveDistance);
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                            MovementPriority::MOVEMENT_FORCED);
            }
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
    if (bot->HasAura(SP_SPELL_ENSNARING_MOSS))
    {
        // Try to dispel the root effect - SAFE PATTERN from HellfireRampartsActions.cpp:219-231
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

bool RokmarEnsnaringMossAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SP_SPELL_ENSNARING_MOSS);
}

// Heal Grievous Wound
bool RokmarGrievousWoundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Grievous Wound removed at 90%+ HP - boss_rokmar_the_crackler.cpp:54-57
    // Check all group members for wound
    Group* group = bot->GetGroup();
    if (group && PlayerbotAI::IsHeal(bot))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive())
            {
                if ((member->HasAura(SP_SPELL_GRIEVOUS_WOUND_N) || member->HasAura(SP_SPELL_GRIEVOUS_WOUND_H)) 
                    && member->GetHealthPct() < 90.0f)
                {
                    // Priority heal wounded targets
                    if (botAI->CanCastSpell(2061, member, false)) // Flash Heal example
                    {
                        return botAI->CastSpell(2061, member);
                    }
                    
                    // Try any heal spell - SAFE PATTERN
                    Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal");
                    if (spellIdsValue)
                    {
                        std::list<uint32> spellIds = spellIdsValue->Get();
                        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
                        {
                            uint32 spellId = *it;
                            if (botAI->CanCastSpell(spellId, member, false))
                            {
                                return botAI->CastSpell(spellId, member);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Self heal if wounded
    if ((bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_N) || bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_H)) 
        && bot->GetHealthPct() < 90.0f)
    {
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal");
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

bool RokmarGrievousWoundAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return (bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_N) || bot->HasAura(SP_SPELL_GRIEVOUS_WOUND_H)) && bot->GetHealthPct() < 90.0f;
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

    // RESEARCHED: Water Spit AoE damage - boss_rokmar_the_crackler.cpp:62-65
    // It's AoE centered on boss, spread out!
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_WATER_SPIT))
    {
        float safeDistance = 20.0f; // Water Spit has large AoE
        float currentDist = bot->GetExactDist(boss);
        
        if (currentDist < safeDistance)
        {
            // Move away from boss quickly
            float angle = boss->GetAngle(bot) + M_PI;
            float moveDistance = safeDistance - currentDist + 3.0f;
            float x = boss->GetPositionX() + cos(angle) * (currentDist + moveDistance);
            float y = boss->GetPositionY() + sin(angle) * (currentDist + moveDistance);
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
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

    return boss->FindCurrentSpellBySpellId(SP_SPELL_WATER_SPIT) && bot->GetExactDist(boss) < 20.0f;
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

    // RESEARCHED: Acid Spray frontal cone - boss_quagmirran.cpp:52-55
    // Random target, cast every 25 seconds
    // Use FindCurrentSpellBySpellId for IMMEDIATE detection
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_ACID_SPRAY))
    {
        // Check if we're in front arc (wider safety margin)
        if (boss->HasInArc(M_PI / 2, bot)) // 90 degree cone for safety
        {
            // EMERGENCY: Move behind boss NOW
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
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

    return boss->FindCurrentSpellBySpellId(SP_SPELL_ACID_SPRAY) && 
           boss->HasInArc(M_PI / 2, bot);
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

    // RESEARCHED: Poison Bolt Volley AoE - boss_quagmirran.cpp:56-59
    // Cast every 24.4 seconds, MUST interrupt immediately
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_POISON_BOLT_VOLLEY))
    {
        // Melee interrupt if in range
        if (bot->IsWithinMeleeRange(boss))
        {
            botAI->InterruptSpell();
            return true;
        }
        
        // Ranged interrupts with priority - SAFE PATTERN
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

bool QuagmirranPoisonBoltVolleyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->FindCurrentSpellBySpellId(SP_SPELL_POISON_BOLT_VOLLEY);
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

    // RESEARCHED: Uppercut knockback - boss_quagmirran.cpp:48-51
    // Cast every 21.8 seconds on tank - position with back to wall
    // CRITICAL: Detect when Uppercut is coming and prepare positioning
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_UPPERCUT))
    {
        // Uppercut about to happen - ensure we have room behind us
        // This is a knockback, so having a wall behind helps
        return false; // Can't really move during cast
    }
    
    // General tank positioning - keep boss faced away from group
    float distance = bot->GetExactDist(boss);
    if (distance > 5.0f)
    {
        // Move closer to maintain threat
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
    return boss && boss->IsAlive() && boss->IsInCombat() && bot->GetExactDist(boss) > 5.0f;
}

// Position behind boss to avoid Cleave
bool QuagmirranCleavePositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Tanks handle cleave, others avoid
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Cleave every 9.1s initially, then 18.8-24.8s - boss_quagmirran.cpp:44-47
    // Non-tanks should stay behind boss at all times
    if (botAI->IsMelee(bot))
    {
        // Check if we're in front arc (dangerous for cleave)
        if (boss->HasInArc(M_PI / 2, bot))
        {
            // Move behind boss
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    // Also move if cleave is being cast RIGHT NOW
    if (boss->FindCurrentSpellBySpellId(SP_SPELL_CLEAVE))
    {
        if (boss->HasInArc(M_PI / 3, bot) && bot->GetExactDist(boss) < 10.0f)
        {
            // Emergency move to side
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            return MoveTo(bot->GetMapId(), x, y, boss->GetPositionZ(), 
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool QuagmirranCleavePositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (botAI->IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_QUAGMIRRAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Useful if melee and in front, or if cleave is being cast
    return (botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot)) ||
           (boss->FindCurrentSpellBySpellId(SP_SPELL_CLEAVE) && boss->HasInArc(M_PI / 3, bot) && bot->GetExactDist(boss) < 10.0f);
}