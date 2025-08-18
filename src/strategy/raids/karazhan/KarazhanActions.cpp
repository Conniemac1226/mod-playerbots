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

bool AttumenAvoidChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Attumen Mounted
    Unit* boss = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!boss)
        return false;

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
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        // Ranged should move further away
        else
        {
            // Move to 30 yards (safe ranged distance)
            float angle = bot->GetAngle(boss);
            float x = bot->GetPositionX() + cos(angle) * 10.0f;  // Move 10 yards further back
            float y = bot->GetPositionY() + sin(angle) * 10.0f;
            float z = bot->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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
                
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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

    // Priority kill order: Healers -> Mana Burners -> Stunners -> DPS warriors
    uint32 addPriority[] = {
        NPC_BARONESS_DOROTHEA,  // Priest (healer)
        NPC_LORD_ROBIN,         // Paladin (healer)
        NPC_LADY_KEIRA,         // Priest (mana burn)
        NPC_LORD_CRISPIN,       // Paladin (stun)
        NPC_LADY_CATRIONA,      // Warrior
        NPC_BARON_RAFE          // Warrior
    };

    // Find highest priority add that's alive
    for (uint32 npcId : addPriority)
    {
        Unit* add = bot->FindNearestCreature(npcId, 100.0f, true);
        if (add && add->IsAlive() && add->IsInCombat())
        {
            if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != add)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(add);
                return true;
            }
        }
    }
    
    return false;
}

bool MoroesFocusAddsAction::isUseful()
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
            return true;
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

    // Melee should stay behind to avoid Gouge
    if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
    {
        if (moroes->HasInArc(M_PI / 2, bot))
        {
            float angle = moroes->GetOrientation() + M_PI;
            float x = moroes->GetPositionX() + cos(angle) * 3.0f;
            float y = moroes->GetPositionY() + sin(angle) * 3.0f;
            float z = moroes->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool MoroesPositionAction::isUseful()
{
    return bot->FindNearestCreature(NPC_MOROES, 100.0f) != nullptr;
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
    return bot->getClass() == CLASS_PRIEST && bot->HasSpell(9484);
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
                
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    
    return false;
}

bool MaidenRepentanceAction::isUseful()
{
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool MaidenHolyGroundAction::isUseful()
{
    return bot->HasAura(SPELL_HOLY_GROUND);
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
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
                
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    
    return false;
}

bool OperaPositionAction::isUseful()
{
    return bot->HasAura(30753); // Red Riding Hood
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
                if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != target)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
                    return true;
                }
            }
            else if (npcId == NPC_TINHEAD && (bot->getClass() == CLASS_MAGE || 
                                               bot->getClass() == CLASS_WARLOCK ||
                                               bot->getClass() == CLASS_PRIEST))
            {
                // Tinhead has high armor - casters priority
                if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != target)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
                    return true;
                }
            }
            else if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != target)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
                return true;
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
        
        if (target && botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
            return true;
        }
    }
    
    // Big Bad Wolf - always focus
    Unit* wolf = bot->FindNearestCreature(NPC_BIG_BAD_WOLF, 100.0f);
    if (wolf && wolf->IsAlive())
    {
        if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != wolf)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(wolf);
            return true;
        }
    }
    
    return false;
}

bool OperaFocusTargetAction::isUseful()
{
    uint32 operaNpcs[] = {
        NPC_DOROTHEE, NPC_ROAR, NPC_STRAWMAN, NPC_TINHEAD, NPC_CRONE,
        NPC_ROMULO, NPC_JULIANNE, NPC_BIG_BAD_WOLF
    };

    for (uint32 npcId : operaNpcs)
    {
        if (bot->FindNearestCreature(npcId, 100.0f, true))
            return true;
    }
    
    return false;
}

// Curator Actions
bool CuratorFlareAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Focus Astral Flares immediately
    Unit* flare = bot->FindNearestCreature(NPC_ASTRAL_FLARE, 100.0f, true);
    if (flare && flare->IsAlive())
    {
        if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != flare)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(flare);
            return true;
        }
    }
    
    return false;
}

bool CuratorFlareAction::isUseful()
{
    return bot->FindNearestCreature(NPC_ASTRAL_FLARE, 100.0f, true) != nullptr;
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
    return bot->HasAura(SPELL_FLAME_WREATH);
}

bool AranBlizzardAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Move out of Blizzard
    if (bot->HasAura(SPELL_CIRCULAR_BLIZZARD))
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
    return bot->HasAura(SPELL_CIRCULAR_BLIZZARD);
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool AranDragonsBreathAction::isUseful()
{
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    return aran && bot->GetDistance(aran) < 15.0f;
}

// Terestian Illhoof Actions
bool IllhoofDemonChainsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Attack Demon Chains to free players
    Unit* chains = bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true);
    if (chains && chains->IsAlive())
    {
        if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != chains)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(chains);
            return true;
        }
    }
    
    return false;
}

bool IllhoofDemonChainsAction::isUseful()
{
    return bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true) != nullptr;
}

bool IllhoofImpsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // AOE the Fiendish Imps
    Unit* imp = bot->FindNearestCreature(NPC_FIENDISH_IMP, 30.0f, true);
    if (imp && imp->IsAlive())
    {
        // Use AOE abilities if available
        if (bot->getClass() == CLASS_MAGE && bot->HasSpell(10161)) // Cone of Cold
        {
            return botAI->CastSpell(10161, imp);
        }
        else if (bot->getClass() == CLASS_WARLOCK && bot->HasSpell(30283)) // Shadowfury
        {
            return botAI->CastSpell(30283, imp);
        }
        // Target imp for single target if no AOE
        else if (botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != imp)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(imp);
            return true;
        }
    }
    
    return false;
}

bool IllhoofImpsAction::isUseful()
{
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

    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (!netherspite)
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
        // Check if we need to rotate (30 second debuff duration)
        if (beamState.redBeamHolder == botGuid)
        {
            if (bot->HasAura(38280) && bot->GetAura(38280)->GetDuration() < 5000) // About to expire
            {
                // Need to swap out
                beamState.redBeamHolder = ObjectGuid::Empty;
                return false;
            }
        }
        else if (beamState.redBeamHolder.IsEmpty() || 
                 (currentTime - beamState.lastBeamSwitch > 30000))
        {
            shouldTakeRed = true;
        }
    }
    // Blue Beam (Dominance) - Healers
    else if (PlayerbotAI::IsHeal(bot))
    {
        if (beamState.blueBeamHolder == botGuid)
        {
            if (bot->HasAura(38281) && bot->GetAura(38281)->GetDuration() < 5000)
            {
                beamState.blueBeamHolder = ObjectGuid::Empty;
                return false;
            }
        }
        else if (beamState.blueBeamHolder.IsEmpty())
        {
            shouldTakeBlue = true;
        }
    }
    // Green Beam (Serenity) - DPS
    else if (botAI->IsDps(bot))
    {
        if (beamState.greenBeamHolder == botGuid)
        {
            if (bot->HasAura(38282) && bot->GetAura(38282)->GetDuration() < 5000)
            {
                beamState.greenBeamHolder = ObjectGuid::Empty;
                return false;
            }
        }
        else if (beamState.greenBeamHolder.IsEmpty())
        {
            shouldTakeGreen = true;
        }
    }
    
    // Position to intercept appropriate beam
    if (shouldTakeRed)
    {
        // Position between red portal and Netherspite
        float x = (NETHERSPITE_RED_PORTAL.GetPositionX() + netherspite->GetPositionX()) / 2.0f;
        float y = (NETHERSPITE_RED_PORTAL.GetPositionY() + netherspite->GetPositionY()) / 2.0f;
        float z = netherspite->GetPositionZ();
        
        if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT))
        {
            beamState.redBeamHolder = botGuid;
            beamState.lastBeamSwitch = currentTime;
            return true;
        }
    }
    else if (shouldTakeBlue)
    {
        // Position between blue portal and Netherspite
        float x = (NETHERSPITE_BLUE_PORTAL.GetPositionX() + netherspite->GetPositionX()) / 2.0f;
        float y = (NETHERSPITE_BLUE_PORTAL.GetPositionY() + netherspite->GetPositionY()) / 2.0f;
        float z = netherspite->GetPositionZ();
        
        if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT))
        {
            beamState.blueBeamHolder = botGuid;
            beamState.lastBeamSwitch = currentTime;
            return true;
        }
    }
    else if (shouldTakeGreen)
    {
        // Position between green portal and Netherspite
        float x = (NETHERSPITE_GREEN_PORTAL.GetPositionX() + netherspite->GetPositionX()) / 2.0f;
        float y = (NETHERSPITE_GREEN_PORTAL.GetPositionY() + netherspite->GetPositionY()) / 2.0f;
        float z = netherspite->GetPositionZ();
        
        if (MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT))
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
    return bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f) != nullptr;
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool NetherspiteVoidZoneAction::isUseful()
{
    return bot->HasAura(SPELL_VOID_ZONE);
}

// Prince Malchezaar Actions
bool MalchezaarInfernalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Avoid Infernals and their Hellfire
    Unit* infernal = bot->FindNearestCreature(NPC_NETHERSPITE_INFERNAL, 20.0f, true);
    if (infernal && infernal->IsAlive())
    {
        // Check cooldown to prevent spam
        if (g_karazhan_lastMoveTime[botGuid] + 1000 > currentTime)
            return false;
            
        // Move away from infernal with wider berth for Hellfire
        float angle = bot->GetAngle(infernal) + M_PI;
        float x = bot->GetPositionX() + cos(angle) * 25.0f; // Increased distance
        float y = bot->GetPositionY() + sin(angle) * 25.0f;
        float z = bot->GetPositionZ();
        
        g_karazhan_lastMoveTime[botGuid] = currentTime;
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    // Also check if Malchezaar is casting Infernal summon
    Unit* malchezaar = bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f);
    if (malchezaar && IsCastingSpell(malchezaar, SPELL_INFERNAL_RELAY))
    {
        // Preemptively spread out
        if (g_karazhan_lastMoveTime[botGuid] + 2000 > currentTime)
            return false;
            
        // Move to a safer position away from raid stack
        float angle = bot->GetOrientation() + frand(-M_PI/3, M_PI/3);
        float x = bot->GetPositionX() + cos(angle) * 15.0f;
        float y = bot->GetPositionY() + sin(angle) * 15.0f;
        float z = bot->GetPositionZ();
        
        g_karazhan_lastMoveTime[botGuid] = currentTime;
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool MalchezaarInfernalAction::isUseful()
{
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
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool MalchezaarEnfeebleAction::isUseful()
{
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool NightbanePositionAction::isUseful()
{
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
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool NightbaneCharredEarthAction::isUseful()
{
    return bot->HasAura(SPELL_CHARRED_EARTH);
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
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
    }
    
    return false;
}

bool NightbaneAirPhaseAction::isUseful()
{
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    return nightbane && !nightbane->IsWithinMeleeRange(bot);
}

// Chess Event Actions
bool ChessEventMoveAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if bot is controlling a chess piece (vehicle)
    Unit* vehicle = bot->GetVehicleBase();
    if (!vehicle)
        return false;
        
    // Identify controlled piece type
    uint32 pieceEntry = vehicle->GetEntry();
    bool isRanged = (pieceEntry == NPC_HUMAN_CONJURER || pieceEntry == NPC_ORC_WARLOCK ||
                     pieceEntry == NPC_HUMAN_CLERIC || pieceEntry == NPC_ORC_NECROLYTE);
    bool isKing = (pieceEntry == NPC_CHESS_KING_LLANE || pieceEntry == NPC_WARCHIEF_BLACKHAND);
    
    // Find enemy king
    Unit* enemyKing = nullptr;
    if (pieceEntry <= NPC_HUMAN_CLERIC) // Human side
        enemyKing = bot->FindNearestCreature(NPC_WARCHIEF_BLACKHAND, 100.0f);
    else // Orc side
        enemyKing = bot->FindNearestCreature(NPC_CHESS_KING_LLANE, 100.0f);
        
    if (!enemyKing)
        return false;
        
    // Movement strategy based on piece type
    float desiredDistance = isRanged ? 20.0f : 5.0f;
    float currentDistance = vehicle->GetDistance(enemyKing);
    
    // King pieces should stay back
    if (isKing)
        desiredDistance = 30.0f;
    
    // Move to optimal range
    if (fabs(currentDistance - desiredDistance) > 3.0f)
    {
        float angle = vehicle->GetAngle(enemyKing);
        float moveDistance = currentDistance > desiredDistance ? -5.0f : 5.0f;
        
        float x = vehicle->GetPositionX() + cos(angle) * moveDistance;
        float y = vehicle->GetPositionY() + sin(angle) * moveDistance;
        float z = vehicle->GetPositionZ();
        
        // Chess pieces move in straight lines
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }
    
    return false;
}

bool ChessEventMoveAction::isUseful()
{
    return bot->GetVehicleBase() != nullptr;
}

bool ChessEventAbilityAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* vehicle = bot->GetVehicleBase();
    if (!vehicle)
        return false;
        
    uint32 pieceEntry = vehicle->GetEntry();
    
    // Use piece abilities based on type
    switch (pieceEntry)
    {
        case NPC_HUMAN_FOOTMAN:
        case NPC_ORC_GRUNT:
            // Heroic Blow / Vicious Strike
            if (Unit* target = vehicle->GetVictim())
            {
                if (vehicle->IsWithinMeleeRange(target))
                    return botAI->CastSpell(32227, target); // Heroic Blow
            }
            break;
            
        case NPC_HUMAN_CHARGER:
        case NPC_ORC_WOLF:
            // Smash / Bite
            if (Unit* target = vehicle->GetVictim())
            {
                if (vehicle->GetDistance(target) < 8.0f)
                    return botAI->CastSpell(32228, target); // Smash
            }
            break;
            
        case NPC_HUMAN_CONJURER:
        case NPC_ORC_WARLOCK:
            // Elemental Blast / Fireball
            if (Unit* target = vehicle->GetVictim())
            {
                if (vehicle->GetDistance(target) < 30.0f)
                    return botAI->CastSpell(32236, target); // Elemental Blast
            }
            break;
            
        case NPC_HUMAN_CLERIC:
        case NPC_ORC_NECROLYTE:
            // Heal / Shadow Mend
            {
                // Find injured ally piece
                std::list<Unit*> allies;
                Acore::AnyFriendlyUnitInObjectRangeCheck u_check(vehicle, vehicle, 30.0f);
                Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(vehicle, allies, u_check);
                Cell::VisitObjects(vehicle, searcher, 30.0f);
                
                for (Unit* ally : allies)
                {
                    if (ally->GetHealthPct() < 50.0f)
                        return botAI->CastSpell(32238, ally); // Heal
                }
            }
            break;
            
        case NPC_CHESS_KING_LLANE:
        case NPC_WARCHIEF_BLACKHAND:
            // Sweep / Cleave
            if (Unit* target = vehicle->GetVictim())
            {
                if (vehicle->IsWithinMeleeRange(target))
                    return botAI->CastSpell(32226, target); // Sweep
            }
            break;
    }
    
    return false;
}

bool ChessEventAbilityAction::isUseful()
{
    return bot->GetVehicleBase() != nullptr;
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
    return botAI->IsTank(bot) && bot->FindNearestCreature(NPC_MOROES, 100.0f) != nullptr;
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
    uint32 botClass = bot->getClass();
    return (botClass == CLASS_PRIEST || botClass == CLASS_PALADIN || 
            botClass == CLASS_SHAMAN || botClass == CLASS_MAGE || 
            botClass == CLASS_DRUID);
}