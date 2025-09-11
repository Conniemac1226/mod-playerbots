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
std::map<ObjectGuid, time_t> groundSlamTimes;

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
    
    // WotLK Standard Pattern: Use disperse distance AI value system
    // This leverages the proven spreading AI used in Ulduar, ICC, etc.
    SET_AI_VALUE(float, "disperse distance", 15.0f); // 15 yards spread for Gruul
    
    return true;
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
    // By this time, bots should already be spread out and will be petrified
    // Clear the spread distance as positioning is complete
    
    // WotLK Standard Pattern: Clear disperse distance after spread complete
    if (AI_VALUE(float, "disperse distance") > 0.0f)
    {
        SET_AI_VALUE(float, "disperse distance", 0.0f);
        return true;
    }
    
    return false;
}

bool GruulShatterPositionAction::isUseful()
{
    // Useful when spread is active and Gruul is present; allow during petrify to clear state
    return bot && bot->IsAlive() &&
           bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f, true) != nullptr &&
           AI_VALUE(float, "disperse distance") > 0.0f;
}

bool GruulCaveInAction::Execute(Event event)
{
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;

    // Prefer a quick sidestep out of the ground effect: move perpendicular
    // to the vector from Gruul to the bot by ~10 yards.
    float angleToGruul = gruul->GetAngle(bot);
    float sidestep = angleToGruul + M_PI_2; // 90 degrees

    Position pos = bot->GetPosition();
    pos.m_positionX += cos(sidestep) * 10.0f;
    pos.m_positionY += sin(sidestep) * 10.0f;

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

    // Off-tanks should remain in melee to soak Hurtful Strike
    if (botAI->IsTank(bot))
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
    // Follow kill order with priority targeting
    for (uint32 npcId : councilKillOrder)
    {
        Unit* target = bot->FindNearestCreature(npcId, 150.0f, true);
        if (target && target->IsAlive() && target->IsInCombat())
        {
            // Use WotLK pattern: AttackAction::Attack()
            return Attack(target);
        }
    }
    
    return false;
}

bool MaulgarPositionAction::Execute(Event event)
{
    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Position behind Maulgar to avoid Arcing Smash
    if (botAI->IsMelee(bot))
    {
        // Move if we are in his frontal arc (danger)
        if (maulgar->HasInArc(M_PI / 2, bot))
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
                     false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
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
    if (maulgar->HasInArc(M_PI / 2, bot))
    {
        float angle = maulgar->GetOrientation() + M_PI;
        Position pos = maulgar->GetPosition();
        pos.m_positionX += cos(angle) * 3.0f;
        pos.m_positionY += sin(angle) * 3.0f;
        
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
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
    if (krosh->HasAura(KROSH_SPELL_SPELLSHIELD))
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
    if (bot->HasAura(KIGGLER_SPELL_GREATER_POLYMORPH))
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

bool OlmWildFelStalkerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot) 
        return false;
        
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    // Anti-ping-pong: Don't switch between Wild Fel Stalkers if already attacking one
    if (currentTarget && currentTarget->GetEntry() == NPC_WILD_FEL_STALKER)
    {
        return false;
    }
    
    // ICC Pattern: Simple spawned add targeting
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (const auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_WILD_FEL_STALKER)
        {
            return Attack(unit); // AttackAction inheritance pattern
        }
    }
    
    return false;
}

bool OlmWildFelStalkerAction::isUseful()
{
    return bot && bot->IsAlive() && !botAI->IsHeal(bot);
}

bool BlindeyeInterruptAction::Execute(Event event)
{
    Unit* blindeye = bot->FindNearestCreature(NPC_BLINDEYE_THE_SEER, 150.0f);
    if (!blindeye)
        return false;
        
    // Interrupt heals
    if (IsCastingSpell(blindeye, BLINDEYE_SPELL_HEAL) || IsCastingSpell(blindeye, BLINDEYE_SPELL_PRAYER_OH))
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

// AoE Avoidance Actions
bool KroshBlastWaveAvoidAction::Execute(Event event)
{
    Unit* krosh = bot->FindNearestCreature(NPC_KROSH_FIREHAND, 150.0f);
    if (!krosh)
        return false;

    // If within Blast Wave range and casting, step out directly away from Krosh
    if (bot->GetDistance(krosh) < 15.0f && IsCastingSpell(krosh, KROSH_SPELL_BLAST_WAVE))
    {
        float angle = krosh->GetAngle(bot) + M_PI;
        Position pos = bot->GetPosition();
        pos.m_positionX += cos(angle) * 15.0f;
        pos.m_positionY += sin(angle) * 15.0f;

        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool KroshBlastWaveAvoidAction::isUseful()
{
    return bot && bot->IsAlive();
}

bool KigglerArcaneExplosionAvoidAction::Execute(Event event)
{
    Unit* kiggler = bot->FindNearestCreature(NPC_KIGGLER_THE_CRAZED, 150.0f);
    if (!kiggler)
        return false;

    // If within Arcane Explosion range and casting, step out
    if (bot->GetDistance(kiggler) < 10.0f && IsCastingSpell(kiggler, KIGGLER_SPELL_ARCANE_EXPLOSION))
    {
        float angle = kiggler->GetAngle(bot) + M_PI;
        Position pos = bot->GetPosition();
        pos.m_positionX += cos(angle) * 12.0f;
        pos.m_positionY += sin(angle) * 12.0f;

        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool KigglerArcaneExplosionAvoidAction::isUseful()
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
    if (Aura* growth = gruul->GetAura(GRUUL_SPELL_GROWTH))
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
    // First: offensive dispel on Blindeye shields if possible (Priest/Shaman)
    {
        uint32 councilIds[] = {
            NPC_HIGH_KING_MAULGAR,
            NPC_KROSH_FIREHAND,
            NPC_OLM_THE_SUMMONER,
            NPC_KIGGLER_THE_CRAZED,
            NPC_BLINDEYE_THE_SEER
        };

        for (uint32 npcId : councilIds)
        {
            if (Unit* council = bot->FindNearestCreature(npcId, 150.0f))
            {
                if (council->HasAura(BLINDEYE_SPELL_GREATER_PW_SHIELD))
                {
                    // Priest Dispel Magic (offensive)
                    if (bot->getClass() == CLASS_PRIEST && botAI->CanCastSpell(988, council))
                    {
                        botAI->CastSpell(988, council);
                        return true;
                    }
                    // Shaman Purge
                    if (bot->getClass() == CLASS_SHAMAN && botAI->CanCastSpell(370, council))
                    {
                        botAI->CastSpell(370, council);
                        return true;
                    }
                }
            }
        }
    }

    // Then: friendly dispel for Greater Polymorph
    if (PlayerbotAI::IsHeal(bot))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(KIGGLER_SPELL_GREATER_POLYMORPH))
            {
                if (botAI->CanCastSpell(988, member)) // Dispel Magic
                {
                    botAI->CastSpell(988, member);
                    return true;
                }
            }
        }
    }

    return false;
}

bool GruulDispelAction::isUseful()
{
    return bot && bot->IsAlive() && PlayerbotAI::IsHeal(bot);
}
