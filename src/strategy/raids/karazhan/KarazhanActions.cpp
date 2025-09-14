#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "MotionMaster.h"
#include "Group.h"
#include "GroupReference.h"
#include "ObjectGuid.h"
#include "Position.h"

// Helper function to check if a unit is casting a specific spell
static bool IsCastingSpell(Unit* unit, uint32 spellId)
{
    if (!unit || !unit->HasUnitState(UNIT_STATE_CASTING))
        return false;
        
    // Check all spell types
    for (uint32 i = CURRENT_MELEE_SPELL; i <= CURRENT_CHANNELED_SPELL; ++i)
    {
        CurrentSpellTypes spellType = CurrentSpellTypes(i);
        if (Spell* spell = unit->GetCurrentSpell(spellType))
        {
            if (spell->m_spellInfo->Id == spellId)
                return true;
        }
    }
    return false;
}

// Per-bot state management for Karazhan encounters
std::map<ObjectGuid, uint32> g_karazhan_lastMoveTime;
std::map<ObjectGuid, bool> g_karazhan_inSafePosition;
std::map<ObjectGuid, uint32> g_karazhan_lastPhaseTime;
// Chess throttles to protect server integrity
static std::map<ObjectGuid, uint32> g_chess_lastMoveTime;
static std::map<ObjectGuid, uint32> g_chess_lastAbilityTime;

bool AttumenAvoidChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Attumen Mounted
    Unit* boss = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!boss)
        return false;

    std::string roleStr = botAI->IsMelee(bot) ? "Melee" : (botAI->IsHeal(bot) ? "Healer" : "Ranged");

    // During mounted phase, stay within 8 yards (melee) or beyond 25 yards (ranged)
    // to avoid charge which targets players between 8-25 yards
    float distance = bot->GetDistance(boss);
    
    // If we're in the danger zone (8-25 yards), move appropriately
    if (distance > 8.0f && distance < 25.0f)
    {
        // Melee should move closer
        if (botAI->IsMelee(bot))
        {
            // Move to 5 yards (safe melee range)
            float angle = bot->GetAngle(boss);
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            float z = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
        // Ranged should move further away
        else
        {
            // Move to 30 yards (safe ranged distance)
            float angle = bot->GetAngle(boss);
            float x = bot->GetPositionX() + cos(angle) * 10.0f;  // Move 10 yards further back
            float y = bot->GetPositionY() + sin(angle) * 10.0f;
            float z = bot->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }
    
    return false;
}

bool AttumenAvoidChargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find the mounted boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!boss || boss->GetEntry() != NPC_ATTUMEN_MOUNTED)
        return false;

    // Check if we're in the danger zone for charge (8-25 yards)
    float distance = bot->GetDistance(boss);
    return (distance > 8.0f && distance < 25.0f);
}

bool AttumenPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Attumen (unmounted)
    Unit* attumen = nullptr;
    Unit* midnight = nullptr;
    
    // Search for both bosses
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED)
            attumen = unit;
        else if (unit->GetEntry() == NPC_MIDNIGHT)
            midnight = unit;
    }

    // During phase 2 (both bosses up), tanks need to separate them
    if (attumen && midnight)
    {
        // Tanks should position bosses apart to avoid cleave hitting both
        if (botAI->IsTank(bot))
        {
            Unit* myTarget = bot->GetVictim();
            if (myTarget)
            {
                // If tanking Attumen, move him away from Midnight
                if (myTarget == attumen)
                {
                    float angle = attumen->GetAngle(midnight) + M_PI;  // Opposite direction from Midnight
                    float x = attumen->GetPositionX() + cos(angle) * 10.0f;
                    float y = attumen->GetPositionY() + sin(angle) * 10.0f;
                    float z = attumen->GetPositionZ();
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
        // Non-tanks should stay behind Attumen to avoid Shadowcleave
        else if (attumen)
        {
            // Position behind Attumen (if we're in front arc, we're in danger)
            if (bot->GetDistance(attumen) < 10.0f && attumen->HasInArc(M_PI / 2, bot))
            {
                float angle = attumen->GetOrientation() + M_PI;
                float x = attumen->GetPositionX() + cos(angle) * 5.0f;
                float y = attumen->GetPositionY() + sin(angle) * 5.0f;
                float z = attumen->GetPositionZ();
                
                return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    
    return false;
}

bool AttumenPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for phase 2 (both bosses up)
    bool hasAttumen = false;
    bool hasMidnight = false;
    
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED)
            hasAttumen = true;
        else if (unit->GetEntry() == NPC_MIDNIGHT)
            hasMidnight = true;
    }
    
    // Useful during phase 2 when both are up
    return hasAttumen && hasMidnight;
}

// Moroes Actions
bool MoroesFocusAddsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Priority kill order (reliable Moroes strat):
    // 1) Shadow Priest (Dorothea)
    // 2) Holy Priest (Catriona)
    // 3) Holy Paladin (Keira)
    // 4) Arms Warrior (Robin)
    // 5) Retribution Paladin (Rafe)
    // 6) Protection Warrior (Crispin)
    uint32 addPriority[] = {
        NPC_BARONESS_DOROTHEA,
        NPC_LADY_CATRIONA,
        NPC_LADY_KEIRA,
        NPC_LORD_ROBIN,
        NPC_BARON_RAFE,
        NPC_LORD_CRISPIN
    };

    // Find highest priority add that's alive
    for (uint32 npcId : addPriority)
    {
        Unit* add = bot->FindNearestCreature(npcId, 100.0f, true);
        if (!add || !add->IsAlive() || !add->IsInCombat())
            continue;

        // Set target for downstream actions and UI selection
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(add);
        bot->SetSelection(add->GetGUID());

        // Melee: if not in melee range, path to target; otherwise attack now
        if (botAI->IsMelee(bot))
        {
            if (!bot->IsWithinMeleeRange(add))
                return ReachCombatTo(add);
            return Attack(add);
        }

        // Ranged/casters
        return Attack(add);
    }
    
    // Fall back to Moroes if no adds
    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f, true);
    if (moroes && moroes->IsAlive())
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(moroes);
        bot->SetSelection(moroes->GetGUID());

        if (botAI->IsMelee(bot))
        {
            if (!bot->IsWithinMeleeRange(moroes))
                return ReachCombatTo(moroes);
            return Attack(moroes);
        }

        return Attack(moroes);
    }
    
    return false;
}


bool MoroesPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f);
    if (!moroes)
        return false;

    float distanceToMoroes = bot->GetDistance(moroes);
    std::string roleStr = botAI->IsMelee(bot) ? "Melee" : (botAI->IsHeal(bot) ? "Healer" : "Ranged");
    
    // Melee should stay behind to avoid Gouge
    if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
    {
        bool inArc = moroes->HasInArc(M_PI / 2, bot);
        if (inArc)
        {
            float angle = moroes->GetOrientation() + M_PI;
            float x = moroes->GetPositionX() + cos(angle) * 3.0f;
            float y = moroes->GetPositionY() + sin(angle) * 3.0f;
            float z = moroes->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }
    // Ranged and healers just need to be in reasonable range to participate
    else if (!botAI->IsMelee(bot))
    {
        if (distanceToMoroes > 25.0f)
        {
            return MoveTo(moroes->GetMapId(), moroes->GetPositionX(), moroes->GetPositionY(), moroes->GetPositionZ(), false, true, false, false);
        }
    }
    
    return false;
}

bool MoroesPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f);
    if (!moroes)
        return false;
    
    // Only useful for positioning if we're in a bad position, not just because boss exists
    // Let normal combat handle basic engagement
    float distance = bot->GetDistance(moroes);
    bool needsReposition = false;
    
    // Melee need repositioning if in front arc (gouge danger)  
    if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
    {
        needsReposition = moroes->HasInArc(M_PI / 2, bot);
    }
    // Ranged need repositioning if too close
    else if (!botAI->IsMelee(bot) && !botAI->IsHeal(bot))
    {
        needsReposition = (distance < 15.0f);
    }
    
    return needsReposition;
}

bool MoroesCrowdControlAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 addIds[] = {
        NPC_BARONESS_DOROTHEA, NPC_LADY_CATRIONA, NPC_LADY_KEIRA,
        NPC_LORD_ROBIN, NPC_LORD_CRISPIN, NPC_BARON_RAFE
    };
    
    // Check class and apply appropriate CC
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
        {
            // Shackle Undead on adds
            for (uint32 npcId : addIds)
            {
                Unit* add = bot->FindNearestCreature(npcId, 30.0f, true);
                if (add && add->IsAlive() && !add->HasAura(9484) && // No Shackle
                    !add->HasAura(118) && !add->HasAura(12826)) // No Poly/Trap
                {
                    return botAI->CastSpell(9484, add); // Shackle Undead
                }
            }
            break;
        }
        
        case CLASS_MAGE:
        {
            // Polymorph on adds
            for (uint32 npcId : addIds)
            {
                Unit* add = bot->FindNearestCreature(npcId, 30.0f, true);
                if (add && add->IsAlive() && !add->HasAura(118) && // No Polymorph
                    !add->HasAura(9484) && !add->HasAura(12826) && // No other CC
                    !add->HasUnitState(UNIT_STATE_CASTING)) // Not casting
                {
                    return botAI->CastSpell(118, add); // Polymorph
                }
            }
            break;
        }
        
        case CLASS_HUNTER:
        {
            // Freezing Trap on adds
            for (uint32 npcId : addIds)
            {
                Unit* add = bot->FindNearestCreature(npcId, 30.0f, true);
                if (add && add->IsAlive() && !add->HasAura(3355) && // No Freezing Trap
                    !add->HasAura(9484) && !add->HasAura(118) && // No other CC
                    add->GetDistance(bot) < 5.0f) // In trap range
                {
                    return botAI->CastSpell(14311, bot); // Freezing Trap
                }
            }
            break;
        }
        
        case CLASS_ROGUE:
        {
            // Sap on adds (before combat)
            if (!bot->IsInCombat())
            {
                for (uint32 npcId : addIds)
                {
                    Unit* add = bot->FindNearestCreature(npcId, 10.0f, true);
                    if (add && add->IsAlive() && !add->IsInCombat() &&
                        !add->HasAura(6770)) // No Sap
                    {
                        return botAI->CastSpell(6770, add); // Sap
                    }
                }
            }
            // Blind in combat
            else
            {
                for (uint32 npcId : addIds)
                {
                    Unit* add = bot->FindNearestCreature(npcId, 10.0f, true);
                    if (add && add->IsAlive() && !add->HasAura(2094) && // No Blind
                        !add->HasAura(9484) && !add->HasAura(118)) // No other CC
                    {
                        return botAI->CastSpell(2094, add); // Blind
                    }
                }
            }
            break;
        }
        
        case CLASS_PALADIN:
        {
            // Turn Undead (fear) on adds
            for (uint32 npcId : addIds)
            {
                Unit* add = bot->FindNearestCreature(npcId, 20.0f, true);
                if (add && add->IsAlive() && !add->HasAura(5484) && // No Turn Undead
                    add->GetHealthPct() < 35.0f) // Low health for fear
                {
                    return botAI->CastSpell(5484, add); // Turn Undead
                }
            }
            break;
        }
        
        case CLASS_WARLOCK:
        {
            // Banish demons (if any) or Fear
            for (uint32 npcId : addIds)
            {
                Unit* add = bot->FindNearestCreature(npcId, 30.0f, true);
                if (add && add->IsAlive() && !add->HasAura(710) && // No Banish
                    !add->HasAura(5782) && !add->HasAura(9484)) // No Fear/Shackle
                {
                    return botAI->CastSpell(5782, add); // Fear
                }
            }
            break;
        }
    }
    
    return false;
}

bool MoroesCrowdControlAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Check if any adds are alive
    uint32 addIds[] = {
        NPC_BARONESS_DOROTHEA, NPC_LADY_CATRIONA, NPC_LADY_KEIRA,
        NPC_LORD_ROBIN, NPC_LORD_CRISPIN, NPC_BARON_RAFE
    };

    for (uint32 npcId : addIds)
    {
        if (bot->FindNearestCreature(npcId, 100.0f, true))
        {
            // Check if bot has appropriate CC abilities
            switch (bot->getClass())
            {
                case CLASS_PRIEST:
                    return bot->HasSpell(9484); // Shackle Undead
                case CLASS_MAGE:
                    return bot->HasSpell(118); // Polymorph
                case CLASS_HUNTER:
                    return bot->HasSpell(14311); // Freezing Trap
                case CLASS_ROGUE:
                    return bot->HasSpell(6770) || bot->HasSpell(2094); // Sap or Blind
                case CLASS_PALADIN:
                    return bot->HasSpell(5484); // Turn Undead
                case CLASS_WARLOCK:
                    return bot->HasSpell(5782); // Fear
                default:
                    return false;
            }
        }
    }
    
    return false;
}

// Maiden of Virtue Actions
bool MaidenRepentanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // If affected by Repentance, need dispel
    if (bot->HasAura(SPELL_REPENTANCE))
    {
        // Call for dispel (handled by healer bots)
        return false;
    }

    // Spread out to avoid chain Repentance
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot && member->GetDistance(bot) < 8.0f)
            {
                // Move away from other players
                float angle = bot->GetAngle(member) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 10.0f;
                float y = bot->GetPositionY() + sin(angle) * 10.0f;
                float z = bot->GetPositionZ();
                
                return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    
    return false;
}

bool MaidenRepentanceAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->FindNearestCreature(NPC_MAIDEN_OF_VIRTUE, 100.0f) != nullptr;
}

bool MaidenHolyGroundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if standing in Holy Ground (consecration)
    if (bot->HasAura(SPELL_HOLY_GROUND))
    {
        // Move out of it
        float angle = bot->GetOrientation();
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool MaidenHolyGroundAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Active when Maiden is present and holy ground effects are active
    Unit* maiden = bot->FindNearestCreature(NPC_MAIDEN_OF_VIRTUE, 100.0f);
    if (!maiden)
        return false;
        
    // Check if maiden is casting holy ground or if bot is affected
    return maiden->FindCurrentSpellBySpellId(SPELL_HOLY_GROUND) || 
           bot->HasAura(SPELL_HOLY_GROUND);
}

// Opera Event Actions
bool OperaPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;


    // Big Bad Wolf Event - Kite if Red Riding Hood
    Unit* wolf = bot->FindNearestCreature(NPC_BIG_BAD_WOLF, 100.0f);
    if (wolf && bot->HasAura(30753)) // Red Riding Hood debuff
    {
        // Run in a circle around the stage to kite
        float centerX = -10883.0f; // Opera stage center
        float centerY = -1762.0f;
        float currentAngle = atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);
        float newAngle = currentAngle + 0.5f; // Move along circle
        
        float x = centerX + cos(newAngle) * 20.0f;
        float y = centerY + sin(newAngle) * 20.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    // Wizard of Oz - Spread for Dorothee's fear
    Unit* dorothee = bot->FindNearestCreature(NPC_DOROTHEE, 100.0f);
    if (dorothee && dorothee->IsAlive())
    {
        // Maintain 10 yard spread for fear
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->GetDistance(bot) < 10.0f)
                {
                    float angle = bot->GetAngle(member) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 5.0f;
                    float y = bot->GetPositionY() + sin(angle) * 5.0f;
                    float z = bot->GetPositionZ();
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }
    
    // Romulo & Julianne - Position tanks on opposite sides
    Unit* romulo = bot->FindNearestCreature(NPC_ROMULO, 100.0f);
    Unit* julianne = bot->FindNearestCreature(NPC_JULIANNE, 100.0f);
    if (romulo && julianne && botAI->IsTank(bot))
    {
        // Tanks should keep them separated
        float distance = romulo->GetDistance(julianne);
        if (distance < 20.0f)
        {
            Unit* myTarget = bot->GetVictim();
            if (myTarget == romulo || myTarget == julianne)
            {
                Unit* otherBoss = (myTarget == romulo) ? julianne : romulo;
                float angle = myTarget->GetAngle(otherBoss) + M_PI;
                float x = myTarget->GetPositionX() + cos(angle) * 10.0f;
                float y = myTarget->GetPositionY() + sin(angle) * 10.0f;
                float z = myTarget->GetPositionZ();
                
                return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    
    return false;
}

bool OperaPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if any Opera boss is present
    uint32 operaNpcs[] = {
        NPC_DOROTHEE, NPC_ROAR, NPC_STRAWMAN, NPC_TINHEAD, NPC_CRONE,
        NPC_ROMULO, NPC_JULIANNE, NPC_BIG_BAD_WOLF
    };

    for (uint32 npcId : operaNpcs)
    {
        Unit* boss = bot->FindNearestCreature(npcId, 100.0f, true);
        if (boss)
            return true;
    }
    
    return false;
}

bool OperaFocusTargetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Wizard of Oz: Kill order with special mechanics
    uint32 ozPriority[] = {
        NPC_DOROTHEE, // Kill first (water bolt interrupter)
        NPC_ROAR,     // Kill second (fear)
        NPC_STRAWMAN, // Kill third (vulnerable to fire)
        NPC_TINHEAD,  // Kill fourth (high armor, use magic)
        NPC_CRONE     // Kill last (chain lightning)
    };

    for (uint32 npcId : ozPriority)
    {
        Unit* target = bot->FindNearestCreature(npcId, 100.0f, true);
        if (target && target->IsAlive())
        {
            // Special mechanics per mob
            if (npcId == NPC_STRAWMAN && bot->getClass() == CLASS_MAGE)
            {
                // Strawman vulnerable to fire - mages priority
                return Attack(target);
            }
            else if (npcId == NPC_TINHEAD && (bot->getClass() == CLASS_MAGE || 
                                               bot->getClass() == CLASS_WARLOCK ||
                                               bot->getClass() == CLASS_PRIEST))
            {
                // Tinhead has high armor - casters priority
                return Attack(target);
            }
            else
            {
                return Attack(target);
            }
        }
    }

    // Romulo & Julianne: Complex kill timing
    Unit* romulo = bot->FindNearestCreature(NPC_ROMULO, 100.0f);
    Unit* julianne = bot->FindNearestCreature(NPC_JULIANNE, 100.0f);
    
    if (romulo && julianne)
    {
        float romuloHealth = romulo->GetHealthPct();
        float julianneHealth = julianne->GetHealthPct();
        
        // Keep their health within 10% of each other
        Unit* target = nullptr;
        if (fabs(romuloHealth - julianneHealth) > 10.0f)
        {
            // Focus the higher health one
            target = (romuloHealth > julianneHealth) ? romulo : julianne;
        }
        else
        {
            // Switch targets periodically to maintain balance
            target = (getMSTime() / 5000 % 2 == 0) ? romulo : julianne;
        }
        
        if (target)
        {
            return Attack(target);
        }
    }
    
    // Big Bad Wolf - always focus
    Unit* wolf = bot->FindNearestCreature(NPC_BIG_BAD_WOLF, 100.0f);
    if (wolf && wolf->IsAlive())
    {
        return Attack(wolf);
    }
    
    return false;
}


// Curator Actions
bool CuratorFlareAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Anti-ping-pong: if attacking flare already, don't switch - CLAUDE.md:722-738
    if (currentTarget && currentTarget->GetEntry() == NPC_ASTRAL_FLARE)
        return false;

    // Find nearest Astral Flare with distance and accessibility checks
    Unit* flare = bot->FindNearestCreature(NPC_ASTRAL_FLARE, 40.0f, true);
    if (flare && flare->IsAlive() && flare->IsInCombat())
    {
        // Ensure flare is accessible and not through walls
        if (!bot->IsWithinLOSInMap(flare))
            return false;
            
        // Validate we can reach the flare (not too far vertically)
        float heightDiff = fabs(bot->GetPositionZ() - flare->GetPositionZ());
        if (heightDiff > 15.0f) // Reasonable height difference
            return false;
            
        // WotLK pattern: AttackAction inheritance enables actual combat
        return Attack(flare);
    }
    
    return false;
}

bool CuratorFlareAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Only useful if Curator is alive and in combat
    Unit* curator = bot->FindNearestCreature(NPC_CURATOR, 100.0f, true);
    if (!curator || !curator->IsAlive() || !curator->IsInCombat())
        return false;
        
    // Check for accessible flares
    Unit* flare = bot->FindNearestCreature(NPC_ASTRAL_FLARE, 40.0f, true);
    return flare && flare->IsAlive() && bot->IsWithinLOSInMap(flare);
}

bool CuratorEvocationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* curator = bot->FindNearestCreature(NPC_CURATOR, 100.0f);
    if (!curator)
        return false;

    // During evocation, burn the boss
    if (curator->HasAura(SPELL_CURATOR_EVOCATION))
    {
        // All DPS focus curator during evocation
        if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != curator)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(curator);
            return true;
        }
    }
    
    return false;
}

bool CuratorEvocationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    Unit* curator = bot->FindNearestCreature(NPC_CURATOR, 100.0f);
    return curator && curator->HasAura(SPELL_CURATOR_EVOCATION);
}

// Shade of Aran Actions
bool AranFlameWreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Check if Aran is casting Flame Wreath
    bool isCasting = false;
    if (aran->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (aran->GetCurrentSpell(spellType))
        {
            uint32 spellId = aran->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_FLAME_WREATH)
                isCasting = true;
        }
    }

    // If affected by Flame Wreath OR it's being cast, stop all movement
    if (bot->HasAura(SPELL_FLAME_WREATH) || isCasting)
    {
        // Stop all movement immediately
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        return true;
    }
    
    return false;
}

bool AranFlameWreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Active when Aran is present and anyone in raid has flame wreath
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;
        
    // Check if any party member has flame wreath or if bot has it
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->HasAura(SPELL_FLAME_WREATH))
                return true;
        }
    }
    
    return bot->HasAura(SPELL_FLAME_WREATH);
}

bool AranBlizzardAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Proactive movement when Aran starts casting OR when already affected
    bool shouldMove = false;
    
    // Check if already affected by blizzard
    if (bot->HasAura(SPELL_CIRCULAR_BLIZZARD))
        shouldMove = true;
    
    // Check if Aran is casting blizzard (proactive movement)
    if (aran->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (aran->GetCurrentSpell(spellType))
        {
            uint32 spellId = aran->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_CIRCULAR_BLIZZARD)
                shouldMove = true;
        }
    }

    if (shouldMove)
    {
        // Move to center of room (safe spot)
        float centerX = -11158.0f;
        float centerY = -1902.0f;
        float centerZ = 232.0f;
        
        return MoveTo(bot->GetMapId(), centerX, centerY, centerZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool AranBlizzardAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Active when Aran is casting or has cast Blizzard
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;
        
    // Check if Aran is casting blizzard or if bot is affected
    return aran->FindCurrentSpellBySpellId(SPELL_CIRCULAR_BLIZZARD) || bot->HasAura(SPELL_CIRCULAR_BLIZZARD);
}

bool AranDragonsBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Move behind Aran to avoid Dragon's Breath frontal cone
    if (bot->GetDistance(aran) < 15.0f && aran->HasInArc(M_PI / 3, bot))
    {
        float angle = aran->GetOrientation() + M_PI;
        float x = aran->GetPositionX() + cos(angle) * 10.0f;
        float y = aran->GetPositionY() + sin(angle) * 10.0f;
        float z = aran->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool AranDragonsBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    return aran && bot->GetDistance(aran) < 15.0f;
}

// Terestian Illhoof Actions
bool IllhoofDemonChainsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Priority targeting: Kilrek > Demon Chains > Illhoof
    // 1. Kilrek (imp companion) - kill first for Broken Pact damage
    Unit* kilrek = bot->FindNearestCreature(NPC_KILTREK, 100.0f, true);
    if (kilrek && kilrek->IsAlive() && kilrek->IsInCombat())
    {
        // Anti-ping-pong: if attacking Kilrek already, don't switch
        if (currentTarget && currentTarget->GetEntry() == NPC_KILTREK)
            return false;
        return Attack(kilrek);
    }
    
    // 2. Demon Chains - emergency priority to free chained players
    Unit* chains = bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true);
    if (chains && chains->IsAlive() && chains->IsInCombat())
    {
        // Anti-ping-pong: if attacking chains already, don't switch
        if (currentTarget && currentTarget->GetEntry() == NPC_DEMON_CHAINS)
            return false;
        return Attack(chains);
    }
    
    return false;
}

bool IllhoofDemonChainsAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true) != nullptr;
}

bool IllhoofImpsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705  
    if (botAI->IsHeal(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Anti-ping-pong: if attacking imp already, don't switch
    if (currentTarget && currentTarget->GetEntry() == NPC_FIENDISH_IMP)
        return false;

    // Target nearest Fiendish Imp
    Unit* imp = bot->FindNearestCreature(NPC_FIENDISH_IMP, 30.0f, true);
    if (imp && imp->IsAlive() && imp->IsInCombat())
    {
        // WotLK pattern: AttackAction inheritance enables actual combat
        return Attack(imp);
    }
    
    return false;
}

bool IllhoofImpsAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->FindNearestCreature(NPC_FIENDISH_IMP, 30.0f, true) != nullptr;
}

// Netherspite Actions
// Beam management state for Netherspite
struct NetherspiteBeamState {
    ObjectGuid redBeamHolder;
    ObjectGuid blueBeamHolder;
    ObjectGuid greenBeamHolder;
    uint32 lastBeamSwitch;
    uint32 beamPhase; // 0 = portal phase, 1 = banish phase
};
static std::map<uint32, NetherspiteBeamState> g_netherspiteBeamState; // Per instance ID

// Beam portal positions (approximate)
static const Position NETHERSPITE_RED_PORTAL = {-11187.0f, -1638.0f, 278.0f};
static const Position NETHERSPITE_BLUE_PORTAL = {-11111.0f, -1638.0f, 278.0f};
static const Position NETHERSPITE_GREEN_PORTAL = {-11149.0f, -1691.0f, 278.0f};

bool NetherspiteBeamAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Throttle beam handling per-bot to avoid movement spam
    static std::map<ObjectGuid, uint32> s_lastBeamTick;
    uint32 nowTick = getMSTime();
    uint32& lastTick = s_lastBeamTick[bot->GetGUID()];
    if (lastTick && nowTick - lastTick < 800) // ~0.8s cadence
        return false;
    lastTick = nowTick;

    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (!netherspite)
        return false;
    // Only operate beam logic during the actual encounter
    if (!netherspite->IsInCombat())
        return false;
        
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    uint32 instanceId = bot->GetInstanceId();
    NetherspiteBeamState& beamState = g_netherspiteBeamState[instanceId];
    
    // Check if we're in banish phase (no beams)
    if (netherspite->HasAura(38524)) // Banish aura
    {
        beamState.beamPhase = 1;
        beamState.redBeamHolder = ObjectGuid::Empty;
        beamState.blueBeamHolder = ObjectGuid::Empty;
        beamState.greenBeamHolder = ObjectGuid::Empty;
        return false;
    }
    
    // Portal phase - manage beams
    if (beamState.beamPhase == 1)
    {
        beamState.beamPhase = 0;
        beamState.lastBeamSwitch = currentTime;
    }
    
    // Determine which beam this bot should take
    bool shouldTakeRed = false;
    bool shouldTakeBlue = false;
    bool shouldTakeGreen = false;
    
    // Red Beam (Perseverance) - Tanks
    if (botAI->IsTank(bot))
    {
        // Check if we need to rotate based on debuff stacks
        if (beamState.redBeamHolder == botGuid)
        {
            // Stay in beam unless we have high stacks or debuff is expiring
            if (bot->HasAura(38280)) 
            {
                Aura* debuff = bot->GetAura(38280);
                if (debuff->GetStackAmount() >= 4 || debuff->GetDuration() < 3000)
                {
                    // Need to swap out - high stacks or about to expire
                    beamState.redBeamHolder = ObjectGuid::Empty;
                    beamState.lastBeamSwitch = currentTime;
                    return false;
                }
                // Continue holding beam
                return false;
            }
        }
        else if (beamState.redBeamHolder.IsEmpty() || 
                 (currentTime - beamState.lastBeamSwitch > 25000)) // Allow rotation after 25s
        {
            shouldTakeRed = true;
        }
    }
    // Blue Beam (Dominance) - Healers
    else if (botAI->IsHeal(bot))
    {
        if (beamState.blueBeamHolder == botGuid)
        {
            // Stay in beam unless debuff is expiring - healers typically don't need to rotate
            if (bot->HasAura(38281))
            {
                Aura* debuff = bot->GetAura(38281);
                if (debuff->GetDuration() < 3000)
                {
                    beamState.blueBeamHolder = ObjectGuid::Empty;
                    beamState.lastBeamSwitch = currentTime;
                    return false;
                }
                // Continue holding beam
                return false;
            }
        }
        else if (beamState.blueBeamHolder.IsEmpty())
        {
            shouldTakeBlue = true;
        }
    }
    // Green Beam (Serenity) - DPS  
    else if (!botAI->IsTank(bot) && !botAI->IsHeal(bot))
    {
        if (beamState.greenBeamHolder == botGuid)
        {
            // Stay in beam unless debuff is expiring - DPS typically don't need to rotate often
            if (bot->HasAura(38282))
            {
                Aura* debuff = bot->GetAura(38282);
                if (debuff->GetDuration() < 3000)
                {
                    beamState.greenBeamHolder = ObjectGuid::Empty;
                    beamState.lastBeamSwitch = currentTime;
                    return false;
                }
                // Continue holding beam
                return false;
            }
        }
        else if (beamState.greenBeamHolder.IsEmpty())
        {
            shouldTakeGreen = true;
        }
    }
    
    // Helper lambda to move toward midpoint of a portal->boss line
    auto moveTowardBeam = [&](const Position& portal) -> bool
    {
        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();

        // Try two points along the beam line: mid and slightly closer to portal
        const float tCandidates[2] = {0.5f, 0.65f};
        for (float t : tCandidates)
        {
            float x = portal.GetPositionX() * t + bx * (1.0f - t);
            float y = portal.GetPositionY() * t + by * (1.0f - t);
            float z = bz;

            // Prefer exact waypoint first, then relaxed
            if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_NORMAL))
                return true;
            if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_NORMAL))
                return true;
        }
        return false;
    };

    // Position to intercept appropriate beam
    if (shouldTakeRed)
    {
        if (moveTowardBeam(NETHERSPITE_RED_PORTAL))
        {
            beamState.redBeamHolder = botGuid;
            beamState.lastBeamSwitch = currentTime;
            return true;
        }
    }
    else if (shouldTakeBlue)
    {
        if (moveTowardBeam(NETHERSPITE_BLUE_PORTAL))
        {
            beamState.blueBeamHolder = botGuid;
            beamState.lastBeamSwitch = currentTime;
            return true;
        }
    }
    else if (shouldTakeGreen)
    {
        if (moveTowardBeam(NETHERSPITE_GREEN_PORTAL))
        {
            beamState.greenBeamHolder = botGuid;
            beamState.lastBeamSwitch = currentTime;
            return true;
        }
    }
    
    return false;
}

bool NetherspiteBeamAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    return netherspite && netherspite->IsInCombat();
}

bool NetherspiteVoidZoneAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Move out of Void Zone
    if (bot->HasAura(SPELL_VOID_ZONE))
    {
        float angle = bot->GetOrientation();
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool NetherspiteVoidZoneAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Active when Netherspite is present and void zones exist
    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (!netherspite || !netherspite->IsInCombat())
        return false;
        
    // Check if bot is in void zone or void zones are nearby
    return bot->HasAura(SPELL_VOID_ZONE) || 
           bot->FindNearestCreature(17470, 30.0f); // Void Zone creature ID
}

// Prince Malchezaar Actions
bool MalchezaarInfernalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Boss combat validation required per CLAUDE.md:590-592
    Unit* malchezaar = bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f, true);
    if (!malchezaar || !malchezaar->IsAlive()) return false;
    
    // Anti-ping-pong: Don't switch between infernals if already attacking one
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_NETHERSPITE_INFERNAL)
    {
        return false;
    }

    // SPAWNED ADD TARGETING: Dynamic targeting per CLAUDE.md:602-608
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (const auto& guid : targets) {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL && unit->IsInCombat()) {
            // WotLK pattern: AttackAction inheritance enables actual combat per CLAUDE.md:607
            return Attack(unit);
        }
    }
    
    // Fallback to main boss when no spawned infernals per CLAUDE.md:612-615
    if (malchezaar && malchezaar->IsAlive()) {
        return Attack(malchezaar);
    }
    return false;
}

bool MalchezaarInfernalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->FindNearestCreature(NPC_NETHERSPITE_INFERNAL, 15.0f, true) != nullptr;
}

bool MalchezaarEnfeebleAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // If affected by Enfeeble, move away from boss
    if (bot->HasAura(30843)) // Enfeeble
    {
        Unit* malchezaar = bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f);
        if (malchezaar)
        {
            // Move to max range
            float angle = bot->GetAngle(malchezaar) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }
    
    return false;
}

bool MalchezaarEnfeebleAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->HasAura(30843); // Enfeeble
}

// Nightbane Actions
bool NightbanePositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;

    // Avoid frontal cone and tail sweep
    float distanceToFront = bot->GetDistance2d(nightbane->GetPositionX() + cos(nightbane->GetOrientation()) * 5.0f,
                                               nightbane->GetPositionY() + sin(nightbane->GetOrientation()) * 5.0f);
    float distanceToBack = bot->GetDistance2d(nightbane->GetPositionX() - cos(nightbane->GetOrientation()) * 5.0f,
                                              nightbane->GetPositionY() - sin(nightbane->GetOrientation()) * 5.0f);

    // Position at the side
    if (distanceToFront < 10.0f || distanceToBack < 10.0f)
    {
        float angle = nightbane->GetOrientation() + (M_PI / 2);
        float x = nightbane->GetPositionX() + cos(angle) * 10.0f;
        float y = nightbane->GetPositionY() + sin(angle) * 10.0f;
        float z = nightbane->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool NightbanePositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f) != nullptr;
}

bool NightbaneCharredEarthAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Move out of Charred Earth zones
    if (bot->HasAura(SPELL_CHARRED_EARTH))
    {
        float angle = bot->GetOrientation();
        float x = bot->GetPositionX() + cos(angle) * 15.0f;
        float y = bot->GetPositionY() + sin(angle) * 15.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool NightbaneCharredEarthAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    // Active when Nightbane is present and charred earth effects are active
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;
        
    // Check if nightbane is casting charred earth or if bot is affected
    return nightbane->FindCurrentSpellBySpellId(SPELL_CHARRED_EARTH) || 
           bot->HasAura(SPELL_CHARRED_EARTH);
}

bool NightbaneAirPhaseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;

    // During air phase, spread out and avoid Rain of Bones
    if (!nightbane->IsWithinMeleeRange(bot))
    {
        // Spread from other players
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->GetDistance(bot) < 10.0f)
                {
                    float angle = bot->GetAngle(member) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 15.0f;
                    float y = bot->GetPositionY() + sin(angle) * 15.0f;
                    float z = bot->GetPositionZ();
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }
    
    return false;
}

bool NightbaneAirPhaseAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    // Only useful when Nightbane is engaged; prevents false positives elsewhere in Karazhan
    return nightbane && nightbane->IsInCombat() && !nightbane->IsWithinMeleeRange(bot);
}

bool NightbaneSkeletonAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Boss combat validation required per CLAUDE.md:590-592
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f, true);
    if (!nightbane || !nightbane->IsAlive()) return false;
    
    // Anti-ping-pong: Don't switch between skeletons if already attacking one
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget && currentTarget->GetEntry() == NPC_RESTLESS_SKELETON)
    {
        return false;
    }

    // SPAWNED ADD TARGETING: Dynamic targeting per CLAUDE.md:602-608
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (const auto& guid : targets) {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_RESTLESS_SKELETON && unit->IsInCombat()) {
            // WotLK pattern: AttackAction inheritance enables actual combat per CLAUDE.md:607
            return Attack(unit);
        }
    }
    
    // Fallback to main boss when no spawned skeletons per CLAUDE.md:612-615
    if (nightbane && nightbane->IsAlive()) {
        return Attack(nightbane);
    }
    return false;
}

bool NightbaneSkeletonAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    // Only useful when Nightbane is present and in combat
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f, true);
    return nightbane && nightbane->IsAlive() && nightbane->IsInCombat();
}

// Chess Event Actions
bool ChessEventMoveAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if bot is controlling a chess piece (vehicle) or charmed piece
    Unit* vehicle = bot->GetVehicleBase();
    Creature* controlledPiece = nullptr;
    if (!vehicle)
    {
        // Try to possess a chess piece if event is active
        Unit* medivh = bot->FindNearestCreature(NPC_ECHO_OF_MEDIVH, 120.0f);
        if (!medivh)
            return false;

        // Skip if bot recently left a piece (server aura blocks immediate re-entry)
        if (bot->HasAura(30529)) // SPELL_RECENTLY_INGAME
            return false;

        // Prefer Alliance pieces (players' side)
        const uint32 pieceIds[] = {
            NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC,
            NPC_HUMAN_CHARGER, NPC_CHESS_KING_LLANE
        };

        for (uint32 pid : pieceIds)
        {
            Creature* piece = bot->FindNearestCreature(pid, 60.0f, true);
            if (!piece || !piece->IsAlive() || piece->GetCharmer())
                continue;

            // Move into gossip range
            if (bot->GetDistance(piece) > 4.5f)
            {
                bot->GetMotionMaster()->MovePoint(0, piece->GetPositionX(), piece->GetPositionY(), piece->GetPositionZ());
                return true;
            }

            // Open gossip and select "Control <piece>"
            {
                WorldPacket hello;
                hello << piece->GetGUID();
                bot->GetSession()->HandleGossipHelloOpcode(hello);

                if (!bot->PlayerTalkClass)
                    return false;

                GossipMenu& menu = bot->PlayerTalkClass->GetGossipMenu();
                uint32 menuId = menu.GetMenuId();

                // Default to first option; prefer one with text starting with "Control"
                int32 selectIndex = -1;
                GossipMenuItemContainer const& items = menu.GetMenuItems();
                for (auto it = items.begin(); it != items.end(); ++it)
                {
                    uint32 idx = it->first;
                    GossipMenuItem const* gi = menu.GetItem(idx);
                    if (!gi)
                        continue;
                    // Heuristic: Chess script sets message to "Control <name>"
                    if (gi->Message.find("Control ") == 0)
                    {
                        selectIndex = static_cast<int32>(idx);
                        break;
                    }
                    if (selectIndex == -1)
                        selectIndex = static_cast<int32>(idx);
                }

                if (selectIndex != -1)
                {
                    std::string code;
                    WorldPacket sel;
                    sel << piece->GetGUID();
                    sel << menuId << static_cast<uint32>(selectIndex);
                    sel << code;
                    bot->GetSession()->HandleGossipSelectOptionOpcode(sel);
                    return true;
                }
            }
        }
        // If we already charmed a piece, pick it up here
        for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
        {
            Creature* c = dynamic_cast<Creature*>(*itr);
            if (c && c->IsAlive() && c->GetCharmerGUID() == bot->GetGUID())
            {
                controlledPiece = c;
                break;
            }
        }
        if (!controlledPiece)
            return false;
    }
        
    // Identify controlled piece type
    Unit* controller = vehicle ? vehicle : static_cast<Unit*>(controlledPiece);
    uint32 pieceEntry = controller->GetEntry();
    bool isRanged = (pieceEntry == NPC_HUMAN_CONJURER || pieceEntry == NPC_ORC_WARLOCK ||
                     pieceEntry == NPC_HUMAN_CLERIC || pieceEntry == NPC_ORC_NECROLYTE);
    bool isKing = (pieceEntry == NPC_CHESS_KING_LLANE || pieceEntry == NPC_WARCHIEF_BLACKHAND);
    
    // Find best target based on piece type and strategy
    Unit* target = nullptr;
    std::vector<uint32> enemyPieceIds;
    
    // Determine enemy pieces based on our side
    if (pieceEntry <= NPC_HUMAN_CLERIC) // Human side - target Orc pieces
    {
        enemyPieceIds = {NPC_ORC_GRUNT, NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE, 
                        NPC_ORC_WOLF, NPC_WARCHIEF_BLACKHAND};
    }
    else // Orc side - target Human pieces
    {
        enemyPieceIds = {NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC,
                        NPC_HUMAN_CHARGER, NPC_CHESS_KING_LLANE};
    }
    
    // Find nearest enemy piece as target
    float closestDistance = 100.0f;
    for (uint32 enemyId : enemyPieceIds)
    {
        Unit* enemy = bot->FindNearestCreature(enemyId, 60.0f);
        if (enemy && enemy->IsAlive())
        {
            float distance = controller->GetDistance(enemy);
            if (distance < closestDistance)
            {
                target = enemy;
                closestDistance = distance;
            }
        }
    }
    
    if (!target)
        return false;
    
    // Movement strategy using chess triggers (cast SPELL_MOVE_GENERIC on best trigger)
    float desiredDistance = isRanged ? 18.0f : (isKing ? 25.0f : 6.0f);
    float currentDistance = controller->GetDistance(target);
    if (fabs(currentDistance - desiredDistance) > 4.0f && controlledPiece)
    {
        // Safety: respect movement cooldown and throttle
        if (controlledPiece->HasAura(KZ_SPELL_MOVE_COOLDOWN) ||
            controlledPiece->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            return false;
        uint32 now = getMSTime();
        uint32& last = g_chess_lastMoveTime[controlledPiece->GetGUID()];
        if (last && now - last < 1500) // 1.5s throttle
            return false;
        std::list<Unit*> nearby;
        Acore::AnyUnitInObjectRangeCheck u_check(controlledPiece, 45.0f);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(controlledPiece, nearby, u_check);
        Cell::VisitObjects(controlledPiece, searcher, 45.0f);

        Creature* bestTrigger = nullptr;
        float bestScore = 1e9f;
        for (Unit* u : nearby)
        {
            Creature* trig = u ? u->ToCreature() : nullptr;
            if (!trig || trig->GetEntry() != KZ_NPC_CHESS_MOVE_TRIGGER)
                continue; // NPC_CHESS_MOVE_TRIGGER
            float dPiece = controlledPiece->GetDistance(trig);
            if (dPiece < 4.0f || dPiece > 22.0f)
                continue;
            float dEnemy = trig->GetDistance(target);
            float score = dEnemy + dPiece * 0.25f;
            if (score < bestScore)
            {
                bestScore = score;
                bestTrigger = trig;
            }
        }
        if (bestTrigger)
        {
            controlledPiece->CastSpell(bestTrigger, KZ_SPELL_MOVE_GENERIC, false);
            g_chess_lastMoveTime[controlledPiece->GetGUID()] = now;
            return true;
        }
    }
    return false;
}

bool ChessEventMoveAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    // Useful if already controlling a piece or the event is around us
    if (bot->GetVehicleBase())
        return true;
    return bot->FindNearestCreature(NPC_ECHO_OF_MEDIVH, 120.0f) != nullptr;
}

bool ChessEventAbilityAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Resolve controlled chess piece (vehicle or charmed creature)
    Creature* piece = nullptr;
    if (Unit* v = bot->GetVehicleBase())
        piece = v->ToCreature();
    if (!piece)
    {
        for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
        {
            Creature* c = dynamic_cast<Creature*>(*itr);
            if (c && c->IsAlive() && c->GetCharmerGUID() == bot->GetGUID())
            {
                piece = c;
                break;
            }
        }
    }
    if (!piece)
        return false;

    // Safety throttles to avoid spam and respect casting state
    if (piece->HasUnitState(UNIT_STATE_CASTING) || piece->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
        return false;
    uint32 now = getMSTime();
    uint32& last = g_chess_lastAbilityTime[piece->GetGUID()];
    if (last && now - last < 1200) // ~GCD throttle
        return false;

    uint32 pieceEntry = piece->GetEntry();

    auto findEnemy = [&](float range) -> Creature*
    {
        std::vector<uint32> enemyIds;
        if (pieceEntry == NPC_HUMAN_FOOTMAN || pieceEntry == NPC_HUMAN_CONJURER || pieceEntry == NPC_HUMAN_CLERIC || pieceEntry == NPC_HUMAN_CHARGER || pieceEntry == NPC_CHESS_KING_LLANE)
            enemyIds = {NPC_ORC_GRUNT, NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE, NPC_ORC_WOLF, NPC_WARCHIEF_BLACKHAND};
        else
            enemyIds = {NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC, NPC_HUMAN_CHARGER, NPC_CHESS_KING_LLANE};

        Creature* best = nullptr; float bestD = 1e9f;
        for (uint32 id : enemyIds)
        {
            Creature* c = piece->FindNearestCreature(id, range, true);
            if (c && c->IsAlive())
            {
                float d = piece->GetDistance(c);
                if (d < bestD) { bestD = d; best = c; }
            }
        }
        return best;
    };

    auto findAllyLow = [&](float range, uint32 hpDiff) -> Creature*
    {
        std::vector<uint32> allyIds;
        if (pieceEntry == NPC_HUMAN_FOOTMAN || pieceEntry == NPC_HUMAN_CONJURER || pieceEntry == NPC_HUMAN_CLERIC || pieceEntry == NPC_HUMAN_CHARGER || pieceEntry == NPC_CHESS_KING_LLANE)
            allyIds = {NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC, NPC_HUMAN_CHARGER, NPC_CHESS_KING_LLANE};
        else
            allyIds = {NPC_ORC_GRUNT, NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE, NPC_ORC_WOLF, NPC_WARCHIEF_BLACKHAND};

        Creature* best = nullptr; uint32 bestDelta = 0;
        for (uint32 id : allyIds)
        {
            Creature* c = piece->FindNearestCreature(id, range, true);
            if (c && c->IsAlive())
            {
                uint32 delta = c->GetMaxHealth() - c->GetHealth();
                if (delta > hpDiff && delta > bestDelta) { bestDelta = delta; best = c; }
            }
        }
        return best;
    };

    switch (pieceEntry)
    {
        case NPC_HUMAN_FOOTMAN: // Pawn A
        {
            if (Creature* e = findEnemy(12.0f))
            {
                piece->CastSpell(e, 37406, false); // Heroic Blow
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            piece->CastSpell(piece, 37414, true); // Shield Block
            g_chess_lastAbilityTime[piece->GetGUID()] = now;
            return true;
        }
        case NPC_HUMAN_CHARGER: // Knight A
        {
            if (Creature* e = findEnemy(10.0f))
            {
                piece->CastSpell(piece, 37498, true); // Stomp
                piece->CastSpell(e, 37453, false);    // Smash
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_HUMAN_CONJURER: // Queen A
        {
            if (Creature* e = findEnemy(25.0f))
            {
                piece->CastSpell(e, 37462, false); // Elemental Blast
                piece->CastSpell(e, 37465, false); // Rain of Fire
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_HUMAN_CLERIC: // Bishop A
        {
            if (Creature* ally = findAllyLow(25.0f, 5000))
            {
                piece->CastSpell(ally, 37455, false); // Healing
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            if (Creature* e = findEnemy(20.0f))
            {
                piece->CastSpell(e, 37459, false); // Holy Lance
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_CHESS_KING_LLANE: // King A
        {
            piece->CastSpell(piece, 37471, true); // Heroism
            if (Creature* e = findEnemy(10.0f))
            {
                piece->CastSpell(piece, 37474, true); // Sweep
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_GRUNT: // Pawn H
        {
            if (Creature* e = findEnemy(12.0f))
            {
                piece->CastSpell(e, 37413, false); // Vicious Strike
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            piece->CastSpell(piece, 37416, true); // Weapon Deflection
            g_chess_lastAbilityTime[piece->GetGUID()] = now;
            return true;
        }
        case NPC_ORC_WOLF: // Knight H
        {
            if (Creature* e = findEnemy(10.0f))
            {
                piece->CastSpell(piece, 37454, true); // Bite
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_WARLOCK: // Queen H
        {
            if (Creature* e = findEnemy(25.0f))
            {
                piece->CastSpell(e, 37463, false); // Fireball
                piece->CastSpell(e, 37469, false); // Poison Cloud
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_NECROLYTE: // Bishop H
        {
            if (Creature* ally = findAllyLow(25.0f, 5000))
            {
                piece->CastSpell(ally, 37456, false); // Shadow Mend
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            if (Creature* e = findEnemy(20.0f))
            {
                piece->CastSpell(e, 37461, false); // Shadow Spear
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_WARCHIEF_BLACKHAND: // King H
        {
            piece->CastSpell(piece, 37472, true); // Bloodlust
            if (Creature* e = findEnemy(10.0f))
            {
                piece->CastSpell(piece, 37476, true); // Cleave
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        default:
            break;
    }
    return false;
}

bool ChessEventAbilityAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    if (bot->GetVehicleBase())
        return true;
    for (Unit::ControlSet::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
    {
        Creature* c = dynamic_cast<Creature*>(*itr);
        if (c && c->IsAlive() && c->GetCharmerGUID() == bot->GetGUID())
            return true;
    }
    return false;
}

// Moroes Tank Swap
bool MoroesTankSwapAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsTank(bot))
        return false;
        
    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f);
    if (!moroes)
        return false;
        
    // Check if current tank has Gouge
    Unit* currentTank = moroes->GetVictim();
    if (currentTank && currentTank->HasAura(SPELL_GOUGE))
    {
        // If we're not the current tank, we need to taunt
        if (currentTank != bot)
        {
            // Taunt based on class
            switch (bot->getClass())
            {
                case CLASS_WARRIOR:
                    return botAI->CastSpell(355, moroes); // Taunt
                case CLASS_PALADIN:
                    return botAI->CastSpell(31789, moroes); // Righteous Defense
                case CLASS_DRUID:
                    return botAI->CastSpell(6795, moroes); // Growl
                case CLASS_DEATH_KNIGHT:
                    return botAI->CastSpell(49576, moroes); // Death Grip
            }
        }
    }
    
    return false;
}

bool MoroesTankSwapAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    return botAI->IsTank(bot) && bot->FindNearestCreature(NPC_MOROES, 100.0f) != nullptr;
}

bool MoroesAttackAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f);
    if (!moroes)
        return false;

    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    // Set target without recursion
    if (currentTarget != moroes)
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(moroes);
    }

    // DON'T call DoNextAction() - that causes infinite recursion
    return false; // Let normal combat actions take over
}

bool MoroesAttackAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_MOROES, 100.0f, true) != nullptr;
}

// Interrupt Rotation System
struct InterruptRotation {
    uint32 lastInterruptTime;
    ObjectGuid lastInterrupter;
    uint32 interruptOrder;
};
static std::map<uint32, InterruptRotation> g_interruptRotation; // Per instance

bool KarazhanInterruptRotationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    uint32 instanceId = bot->GetInstanceId();
    InterruptRotation& rotation = g_interruptRotation[instanceId];
    uint32 currentTime = getMSTime();
    
    // Find targets that need interrupting
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 30.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 30.0f);
    
    for (Unit* target : targets)
    {
        if (!target || !target->IsAlive() || !target->IsHostileTo(bot))
            continue;
            
        // Check if casting an interruptible spell
        if (target->HasUnitState(UNIT_STATE_CASTING))
        {
            // Priority interrupts for specific spells
            bool shouldInterrupt = false;
            uint32 npcId = target->GetEntry();
            
            // Shade of Aran - interrupt Arcane Missiles
            if (npcId == NPC_SHADE_OF_ARAN && IsCastingSpell(target, SPELL_ARCANE_MISSILES))
                shouldInterrupt = true;
                
            // Moroes adds - interrupt heals
            if ((npcId == NPC_BARONESS_DOROTHEA || npcId == NPC_LORD_ROBIN) &&
                target->HasUnitState(UNIT_STATE_CASTING))
                shouldInterrupt = true;
                
            // Opera Dorothee - interrupt Water Bolt
            if (npcId == NPC_DOROTHEE && target->HasUnitState(UNIT_STATE_CASTING))
                shouldInterrupt = true;
                
            if (shouldInterrupt)
            {
                // Check if it's our turn in rotation
                if (currentTime - rotation.lastInterruptTime > 2000 || // 2 sec rotation
                    rotation.lastInterrupter == bot->GetGUID())
                {
                    bool interrupted = false;
                    
                    // Use class interrupt
                    switch (bot->getClass())
                    {
                        case CLASS_WARRIOR:
                            interrupted = botAI->CastSpell(6552, target); // Pummel
                            break;
                        case CLASS_ROGUE:
                            interrupted = botAI->CastSpell(1766, target); // Kick
                            break;
                        case CLASS_MAGE:
                            interrupted = botAI->CastSpell(2139, target); // Counterspell
                            break;
                        case CLASS_SHAMAN:
                            interrupted = botAI->CastSpell(8042, target); // Earth Shock
                            break;
                        case CLASS_DEATH_KNIGHT:
                            interrupted = botAI->CastSpell(47528, target); // Mind Freeze
                            break;
                    }
                    
                    if (interrupted)
                    {
                        rotation.lastInterruptTime = currentTime;
                        rotation.lastInterrupter = bot->GetGUID();
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool KarazhanInterruptRotationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    // Only classes with interrupts
    uint32 botClass = bot->getClass();
    return (botClass == CLASS_WARRIOR || botClass == CLASS_ROGUE || 
            botClass == CLASS_MAGE || botClass == CLASS_SHAMAN || 
            botClass == CLASS_DEATH_KNIGHT);
}

// Dispel Priority System
bool KarazhanDispelAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Priority dispel list
    struct DispelPriority {
        uint32 spellId;
        uint32 priority; // Lower is higher priority
    };
    
    DispelPriority priorityList[] = {
        {SPELL_GARROTE, 1},           // Moroes - Garrote (highest)
        {SPELL_REPENTANCE, 2},        // Maiden - Repentance
        {SPELL_MASS_POLYMORPH, 3},    // Aran - Mass Polymorph
        {30753, 4},                   // Opera - Red Riding Hood
        {SPELL_BLIND, 5},             // Moroes - Blind
        {30843, 6},                   // Malchezaar - Enfeeble
        {SPELL_INTANGIBLE_PRESENCE, 7} // Attumen - Fear
    };
    
    // Find group members with debuffs
    Group* group = bot->GetGroup();
    if (!group)
        return false;
        
    Unit* bestTarget = nullptr;
    uint32 bestPriority = 999;
    
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetDistance(bot) > 40.0f)
            continue;
            
        // Check for priority debuffs
        for (auto& dispel : priorityList)
        {
            if (member->HasAura(dispel.spellId) && dispel.priority < bestPriority)
            {
                bestTarget = member;
                bestPriority = dispel.priority;
            }
        }
    }
    
    if (bestTarget)
    {
        // Dispel based on class
        switch (bot->getClass())
        {
            case CLASS_PRIEST:
                return botAI->CastSpell(988, bestTarget); // Dispel Magic
            case CLASS_PALADIN:
                return botAI->CastSpell(4987, bestTarget); // Cleanse
            case CLASS_SHAMAN:
                return botAI->CastSpell(526, bestTarget); // Cure Poison
            case CLASS_MAGE:
                return botAI->CastSpell(475, bestTarget); // Remove Curse
            case CLASS_DRUID:
                return botAI->CastSpell(2893, bestTarget); // Abolish Poison
        }
    }
    
    return false;
}

bool KarazhanDispelAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    uint32 botClass = bot->getClass();
    return (botClass == CLASS_PRIEST || botClass == CLASS_PALADIN || 
            botClass == CLASS_SHAMAN || botClass == CLASS_MAGE || 
            botClass == CLASS_DRUID);
}
