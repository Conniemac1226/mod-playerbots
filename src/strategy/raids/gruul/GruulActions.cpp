#include "GruulActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Group.h"
#include "GroupReference.h"
#include <map>
#include <vector>

// Track Ground Slam timing for shatter positioning
static std::map<ObjectGuid, time_t> groundSlamTimes;

// Track kill order for council
static std::vector<uint32> councilKillOrder = {
    NPC_BLINDEYE_THE_SEER,    // Kill healer first
    NPC_OLM_THE_SUMMONER,      // Kill summoner second
    NPC_KIGGLER_THE_CRAZED,    // Kill poly caster third
    NPC_KROSH_FIREHAND,        // Kill mage fourth
    NPC_HIGH_KING_MAULGAR      // Kill boss last
};

// Helper function to check if a unit is casting a specific spell
static bool IsCastingSpell(Unit* unit, uint32 spellId)
{
    if (!unit || !unit->HasUnitState(UNIT_STATE_CASTING))
        return false;
        
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

// Gruul the Dragonkiller Actions
bool GruulGroundSlamAction::Execute(Event event)
{
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // CRITICAL: Spread IMMEDIATELY on Ground Slam detection!
    // We have 9.7 seconds before Shatter - this is our ONLY chance to position
    groundSlamTimes[bot->GetGUID()] = time(nullptr);
    
    // Find optimal spread position away from other players
    Group* group = bot->GetGroup();
    if (group)
    {
        Position bestPos;
        float bestMinDist = 0.0f;
        float spreadDistance = 30.0f; // Maximum spread distance
        
        // Test 8 radial positions around Gruul
        for (int i = 0; i < 8; ++i)
        {
            float angle = 2 * M_PI * i / 8.0f;
            Position testPos = gruul->GetPosition();
            testPos.m_positionX += cos(angle) * spreadDistance;
            testPos.m_positionY += sin(angle) * spreadDistance;
            
            // Calculate minimum distance to other group members at this position
            float minDistToOthers = 100.0f;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = member->GetDistance2d(testPos.m_positionX, testPos.m_positionY);
                    if (dist < minDistToOthers)
                        minDistToOthers = dist;
                }
            }
            
            // Pick position with maximum distance from nearest player
            if (minDistToOthers > bestMinDist)
            {
                bestMinDist = minDistToOthers;
                bestPos = testPos;
            }
        }
        
        if (bestMinDist > 0.0f)
        {
            return MoveTo(bot->GetMapId(), bestPos.m_positionX, bestPos.m_positionY, bestPos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // Fallback: spread to random position if no group or calculation fails
    float angle = 2 * M_PI * rand_norm();
    float distance = 25.0f + frand(0.0f, 10.0f);
    Position pos = gruul->GetPosition();
    pos.m_positionX += cos(angle) * distance;
    pos.m_positionY += sin(angle) * distance;
    
    return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool GruulGroundSlamAction::isUseful()
{
    return bot && bot->IsAlive() && !bot->IsNonMeleeSpellCast(false);
}

bool GruulShatterPositionAction::Execute(Event event)
{
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Shatter happens 9.7 seconds after Ground Slam
    // Damage is based on distance between players
    // Spread out as much as possible
    
    float maxDistance = 30.0f;
    float angle = 2 * M_PI * rand_norm();
    float distance = 25.0f + frand(0.0f, 5.0f);
    
    Position pos = gruul->GetPosition();
    pos.m_positionX += cos(angle) * distance;
    pos.m_positionY += sin(angle) * distance;
    
    // Find a position away from other players
    Group* group = bot->GetGroup();
    if (group)
    {
        Position bestPos = pos;
        float bestMinDist = 0.0f;
        
        for (int i = 0; i < 8; ++i)
        {
            angle = 2 * M_PI * i / 8.0f;
            pos = gruul->GetPosition();
            pos.m_positionX += cos(angle) * distance;
            pos.m_positionY += sin(angle) * distance;
            
            float minDist = 100.0f;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = member->GetDistance2d(pos.m_positionX, pos.m_positionY);
                    if (dist < minDist)
                        minDist = dist;
                }
            }
            
            if (minDist > bestMinDist)
            {
                bestMinDist = minDist;
                bestPos = pos;
            }
        }
        
        return MoveTo(bot->GetMapId(), bestPos.m_positionX, bestPos.m_positionY, bestPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool GruulShatterPositionAction::isUseful()
{
    // DEPRECATED: Shatter positioning now handled by Ground Slam action
    // By the time Shatter happens, players are petrified and cannot move
    // This action kept for compatibility but should rarely trigger
    return bot && bot->IsAlive() && !bot->HasAura(SPELL_STONED) && 
           bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f, true) != nullptr;
}

bool GruulCaveInAction::Execute(Event event)
{
    // Cave In targets random locations
    // Move away from any cave in visual effects
    // This would require detecting ground effects
    
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Move to a safer position near the wall
    float angle = gruul->GetAngle(bot);
    float distance = 25.0f;
    
    Position pos = gruul->GetPosition();
    pos.m_positionX += cos(angle) * distance;
    pos.m_positionY += sin(angle) * distance;
    
    return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                 false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
}

bool GruulCaveInAction::isUseful()
{
    return bot && bot->IsAlive();
}

bool GruulHurtfulStrikeAction::Execute(Event event)
{
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // If we're not the main tank and in melee range, move out
    if (gruul->GetVictim() != bot && bot->GetDistance(gruul) <= 5.0f)
    {
        // Check if we're the highest health melee (excluding main tank)
        Group* group = bot->GetGroup();
        if (group)
        {
            bool highestHealth = true;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->GetDistance(gruul) <= 5.0f)
                {
                    if (member->GetHealth() > bot->GetHealth() && gruul->GetVictim() != member)
                    {
                        highestHealth = false;
                        break;
                    }
                }
            }
            
            if (highestHealth)
            {
                // Move out of melee range
                float angle = gruul->GetAngle(bot) + M_PI;
                Position pos = bot->GetPosition();
                pos.m_positionX += cos(angle) * 10.0f;
                pos.m_positionY += sin(angle) * 10.0f;
                
                return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                             false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    
    return false;
}

bool GruulHurtfulStrikeAction::isUseful()
{
    return bot && bot->IsAlive() && botAI->IsMelee(bot);
}

// High King Maulgar Actions
bool MaulgarFocusTargetAction::Execute(Event event)
{
    // Follow kill order
    for (uint32 npcId : councilKillOrder)
    {
        if (Unit* target = bot->FindNearestCreature(npcId, 150.0f, true))
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
            return true;
        }
    }
    
    return false;
}

bool MaulgarFocusTargetAction::isUseful()
{
    return bot && bot->IsAlive();
}

bool MaulgarPositionAction::Execute(Event event)
{
    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Position behind Maulgar to avoid Arcing Smash
    if (botAI->IsMelee(bot))
    {
        if (!maulgar->HasInArc(M_PI / 2, bot))
        {
            float angle = maulgar->GetOrientation() + M_PI;
            Position pos = maulgar->GetPosition();
            pos.m_positionX += cos(angle) * 3.0f;
            pos.m_positionY += sin(angle) * 3.0f;
            
            return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }
    
    return false;
}

bool MaulgarPositionAction::isUseful()
{
    return bot && bot->IsAlive() && botAI->IsMelee(bot);
}

bool MaulgarWhirlwindAction::Execute(Event event)
{
    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Run away from Whirlwind
    if (bot->GetDistance(maulgar) < 10.0f)
    {
        float angle = maulgar->GetAngle(bot) + M_PI;
        Position pos = bot->GetPosition();
        pos.m_positionX += cos(angle) * 15.0f;
        pos.m_positionY += sin(angle) * 15.0f;
        
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool MaulgarWhirlwindAction::isUseful()
{
    return bot && bot->IsAlive() && botAI->IsMelee(bot);
}

bool MaulgarArcingSmashAction::Execute(Event event)
{
    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Move behind Maulgar to avoid frontal cleave
    if (!maulgar->HasInArc(M_PI / 2, bot))
    {
        float angle = maulgar->GetOrientation() + M_PI;
        Position pos = maulgar->GetPosition();
        pos.m_positionX += cos(angle) * 3.0f;
        pos.m_positionY += sin(angle) * 3.0f;
        
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool MaulgarArcingSmashAction::isUseful()
{
    return bot && bot->IsAlive() && botAI->IsMelee(bot);
}

// Council Member Actions
bool KroshSpellstealAction::Execute(Event event)
{
    if (bot->getClass() != CLASS_MAGE)
        return false;
        
    Unit* krosh = bot->FindNearestCreature(NPC_KROSH_FIREHAND, 150.0f);
    if (!krosh)
        return false;
        
    // Mage should spellsteal the shield
    if (krosh->HasAura(SPELL_SPELLSHIELD))
    {
        if (botAI->CanCastSpell(30449, krosh)) // Spellsteal spell ID
        {
            botAI->CastSpell(30449, krosh);
            return true;
        }
    }
    
    return false;
}

bool KroshSpellstealAction::isUseful()
{
    return bot && bot->IsAlive() && bot->getClass() == CLASS_MAGE;
}

bool KigglerPolymorphAction::Execute(Event event)
{
    // Dispel polymorph if possible
    if (bot->HasAura(SPELL_GREATER_POLYMORPH))
    {
        // Request dispel from healers
        return true;
    }
    
    return false;
}

bool KigglerPolymorphAction::isUseful()
{
    return bot && bot->IsAlive();
}

bool OlmPetAction::Execute(Event event)
{
    // Focus Wild Fel Stalker pets
    if (Unit* pet = bot->FindNearestCreature(NPC_WILD_FEL_STALKER, 100.0f, true))
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(pet);
        return true;
    }
    
    return false;
}

bool OlmPetAction::isUseful()
{
    return bot && bot->IsAlive();
}

bool BlindeyeInterruptAction::Execute(Event event)
{
    Unit* blindeye = bot->FindNearestCreature(NPC_BLINDEYE_THE_SEER, 150.0f);
    if (!blindeye)
        return false;
        
    // Interrupt heals
    if (IsCastingSpell(blindeye, SPELL_HEAL) || IsCastingSpell(blindeye, SPELL_PRAYER_OH))
    {
        // Use class interrupt abilities
        uint32 interruptSpells[] = {
            2139,  // Counterspell (Mage)
            1766,  // Kick (Rogue)
            6552,  // Pummel (Warrior)
            19647, // Spell Lock (Warlock)
            147,   // Shield Bash (Warrior)
            72,    // Shield Bash (Warrior - Rank 1)
            15487  // Silence (Priest)
        };
        
        for (uint32 spellId : interruptSpells)
        {
            if (botAI->CanCastSpell(spellId, blindeye))
            {
                botAI->CastSpell(spellId, blindeye);
                return true;
            }
        }
    }
    
    return false;
}

bool BlindeyeInterruptAction::isUseful()
{
    return bot && bot->IsAlive();
}

// Utility Actions
bool GruulTankSwapAction::Execute(Event event)
{
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Tank swap for Growth stacks
    if (Aura* growth = gruul->GetAura(SPELL_GROWTH))
    {
        uint32 stacks = growth->GetStackAmount();
        if (stacks >= 15 && botAI->IsTank(bot))
        {
            // Taunt if we're the off-tank
            if (gruul->GetVictim() != bot)
            {
                // Use taunt abilities
                uint32 tauntSpells[] = {
                    355,   // Taunt (Warrior)
                    62124, // Hand of Reckoning (Paladin)
                    49576, // Death Grip (Death Knight)
                    56222, // Dark Command (Death Knight)
                    6795,  // Growl (Druid)
                    31789  // Righteous Defense (Paladin)
                };
                
                for (uint32 spellId : tauntSpells)
                {
                    if (botAI->CanCastSpell(spellId, gruul))
                    {
                        botAI->CastSpell(spellId, gruul);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool GruulTankSwapAction::isUseful()
{
    return bot && bot->IsAlive() && botAI->IsTank(bot);
}

bool GruulDispelAction::Execute(Event event)
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;
        
    // Dispel priority debuffs
    Group* group = bot->GetGroup();
    if (!group)
        return false;
        
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
            
        // Dispel Greater Polymorph
        if (member->HasAura(SPELL_GREATER_POLYMORPH))
        {
            if (botAI->CanCastSpell(988, member)) // Dispel Magic
            {
                botAI->CastSpell(988, member);
                return true;
            }
        }
    }
    
    return false;
}

bool GruulDispelAction::isUseful()
{
    return bot && bot->IsAlive() && PlayerbotAI::IsHeal(bot);
}