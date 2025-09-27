#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotMgr.h"
#include "AiObjectContext.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "MotionMaster.h"
#include "Group.h"
#include "GroupReference.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "Value.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <limits>
#include <list>
#include <map>
#include <tuple>
#include <unordered_set>
#include <vector>
 

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

namespace
{
    GuidVector FetchNearbyNpcGuids(PlayerbotAI* botAI, char const* cacheName)
    {
        if (!botAI)
            return {};

        if (AiObjectContext* context = botAI->GetAiObjectContext())
        {
            if (Value<GuidVector>* cache = context->GetValue<GuidVector>(cacheName))
                return cache->Get();
        }

        return {};
    }

    template <typename Predicate>
    void ForEachNearbyNpc(PlayerbotAI* botAI, float maxDistance, Predicate&& predicate)
    {
        if (!botAI)
            return;

        Player* bot = botAI->GetBot();
        if (!bot)
            return;

        std::unordered_set<ObjectGuid> seen;

        auto visitCache = [&](char const* cacheName)
        {
            GuidVector guids = FetchNearbyNpcGuids(botAI, cacheName);
            for (ObjectGuid const& guid : guids)
            {
                if (!seen.insert(guid).second)
                    continue;

                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;

                if (maxDistance > 0.0f && bot->GetDistance(unit) > maxDistance)
                    continue;

                predicate(unit);
            }
        };

        visitCache("nearest hostile npcs");
        visitCache("nearest friendly npcs");
    }

    Unit* FindClosestNpcByEntry(PlayerbotAI* botAI, uint32 entry, float maxDistance)
    {
        Unit* closest = nullptr;
        float closestDist = maxDistance;

        ForEachNearbyNpc(botAI, maxDistance, [&](Unit* unit)
        {
            if (unit->GetEntry() != entry)
                return;

            Player* bot = botAI->GetBot();
            if (!bot)
                return;

            float distance = bot->GetDistance(unit);
            if (distance <= closestDist)
            {
                closestDist = distance;
                closest = unit;
            }
        });

        return closest;
    }

    template <typename Predicate>
    bool AnyNearbyChessPiece(PlayerbotAI* botAI, Predicate&& predicate)
    {
        bool found = false;
        ForEachNearbyNpc(botAI, 120.0f, [&](Unit* unit)
        {
            if (found)
                return;

            if (Creature* c = unit->ToCreature())
            {
                if (predicate(c))
                    found = true;
            }
        });
        return found;
    }
}

namespace
{
    struct ChessTargetPriority
    {
        uint32 entry;
        uint8 priority;
    };

    inline bool IsHumanChessPiece(uint32 entry)
    {
        switch (entry)
        {
            case NPC_HUMAN_FOOTMAN:
            case NPC_HUMAN_CHARGER:
            case NPC_HUMAN_CONJURER:
            case NPC_HUMAN_CLERIC:
            case NPC_CHESS_KING_LLANE:
                return true;
            default:
                return false;
        }
    }

    inline std::array<ChessTargetPriority, 5> const& EnemyPriorityList(uint32 pieceEntry)
    {
        static constexpr std::array<ChessTargetPriority, 5> humanVsOrc = {
            ChessTargetPriority{NPC_ORC_NECROLYTE, 95},
            ChessTargetPriority{NPC_ORC_WARLOCK,    90},
            ChessTargetPriority{NPC_ORC_WOLF,       75},
            ChessTargetPriority{NPC_ORC_GRUNT,      60},
            ChessTargetPriority{NPC_WARCHIEF_BLACKHAND, 85}
        };

        static constexpr std::array<ChessTargetPriority, 5> orcVsHuman = {
            ChessTargetPriority{NPC_HUMAN_CLERIC,   95},
            ChessTargetPriority{NPC_HUMAN_CONJURER, 90},
            ChessTargetPriority{NPC_HUMAN_CHARGER,  75},
            ChessTargetPriority{NPC_HUMAN_FOOTMAN,  60},
            ChessTargetPriority{NPC_CHESS_KING_LLANE, 85}
        };

        if (IsHumanChessPiece(pieceEntry))
            return humanVsOrc;
        return orcVsHuman;
    }

    inline std::array<ChessTargetPriority, 5> const& AllyPriorityList(uint32 pieceEntry)
    {
        static constexpr std::array<ChessTargetPriority, 5> humanAllies = {
            ChessTargetPriority{NPC_HUMAN_CHARGER,  90},
            ChessTargetPriority{NPC_HUMAN_CONJURER, 85},
            ChessTargetPriority{NPC_HUMAN_CLERIC,   80},
            ChessTargetPriority{NPC_HUMAN_FOOTMAN,  70},
            ChessTargetPriority{NPC_CHESS_KING_LLANE, 95}
        };

        static constexpr std::array<ChessTargetPriority, 5> orcAllies = {
            ChessTargetPriority{NPC_ORC_WOLF,       90},
            ChessTargetPriority{NPC_ORC_WARLOCK,    85},
            ChessTargetPriority{NPC_ORC_NECROLYTE,  80},
            ChessTargetPriority{NPC_ORC_GRUNT,      70},
            ChessTargetPriority{NPC_WARCHIEF_BLACKHAND, 95}
        };

        if (IsHumanChessPiece(pieceEntry))
            return humanAllies;
        return orcAllies;
    }

    Creature* SelectChessEnemy(Creature* piece, float maxRange)
    {
        if (!piece)
            return nullptr;

        auto const& priorities = EnemyPriorityList(piece->GetEntry());
        uint32 enemyKingEntry = IsHumanChessPiece(piece->GetEntry()) ? NPC_WARCHIEF_BLACKHAND : NPC_CHESS_KING_LLANE;

        if (Creature* king = piece->FindNearestCreature(enemyKingEntry, maxRange, true))
        {
            if (king->IsAlive() && king->GetHealthPct() < 40.0f)
                return king;
        }

        Creature* best = nullptr;
        float bestScore = -std::numeric_limits<float>::max();

        for (ChessTargetPriority const& target : priorities)
        {
            Creature* enemy = piece->FindNearestCreature(target.entry, maxRange, true);
            if (!enemy || !enemy->IsAlive())
                continue;

            float distance = piece->GetDistance(enemy);
            float score = static_cast<float>(target.priority) - distance;
            if (score > bestScore)
            {
                bestScore = score;
                best = enemy;
            }
        }

        return best;
    }

    Creature* SelectChessAllyForHeal(Creature* piece, float maxRange, uint32 minMissingHealth)
    {
        if (!piece)
            return nullptr;

        auto const& allies = AllyPriorityList(piece->GetEntry());
        Creature* best = nullptr;
        float bestScore = -std::numeric_limits<float>::max();

        for (ChessTargetPriority const& proto : allies)
        {
            Creature* ally = piece->FindNearestCreature(proto.entry, maxRange, true);
            if (!ally || !ally->IsAlive() || ally == piece)
                continue;

            uint32 missing = ally->GetMaxHealth() - ally->GetHealth();
            if (missing < minMissingHealth)
                continue;

            float distance = piece->GetDistance(ally);
            float score = static_cast<float>(proto.priority) + (missing / 1000.0f) - distance;
            if (score > bestScore)
            {
                bestScore = score;
                best = ally;
            }
        }

        return best;
    }
}

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

    Unit* attumen = FindClosestNpcByEntry(botAI, NPC_ATTUMEN_UNMOUNTED, 100.0f);
    Unit* midnight = FindClosestNpcByEntry(botAI, NPC_MIDNIGHT, 100.0f);

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

    Unit* attumen = FindClosestNpcByEntry(botAI, NPC_ATTUMEN_UNMOUNTED, 100.0f);
    Unit* midnight = FindClosestNpcByEntry(botAI, NPC_MIDNIGHT, 100.0f);

    return attumen && midnight;
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
namespace
{
    static std::map<ObjectGuid, uint32> g_netherspiteBeamMoveTimes;
    static std::map<ObjectGuid, bool> g_netherspiteLastMoveSideways;
    static std::map<ObjectGuid, bool> g_netherspiteWasBlockingBlueBeam;
    static std::map<ObjectGuid, bool> g_netherspiteWasBlockingGreenBeam;

    static bool IsNetherspiteBanished(Unit* boss)
    {
        return boss && boss->HasAura(SPELL_NETHERSPITE_BANISHED);
    }
}

class KarazhanNetherspiteHelper
{
public:
    KarazhanNetherspiteHelper(Player* bot, PlayerbotAI* botAI) : m_bot(bot), m_botAI(botAI) {}

    Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss) const
    {
        if (!boss || !portal)
            return Position();

        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float bz = boss->GetPositionZ();
        float px = portal->GetPositionX();
        float py = portal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = std::sqrt(dx * dx + dy * dy);

        if (length == 0.0f)
            return Position(bx, by, bz);

        dx /= length;
        dy /= length;

        float targetX = bx + dx * distanceFromBoss;
        float targetY = by + dy * distanceFromBoss;
        return Position(targetX, targetY, bz);
    }

    std::vector<Player*> GetBlueBlockers() const
    {
        std::vector<Player*> blueBlockers;
        if (Group* group = m_bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                    continue;

                bool isDps = m_botAI->IsDps(member);
                bool isWarrior = member->getClass() == CLASS_WARRIOR;
                bool isRogue = member->getClass() == CLASS_ROGUE;
                bool hasExhaustion = member->HasAura(SPELL_EXHAUSTION_DOMINANCE);
                Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
                bool overStack = blueBuff && blueBuff->GetStackAmount() >= 25;

                if (isDps && !isWarrior && !isRogue && !hasExhaustion && !overStack)
                    blueBlockers.push_back(member);
            }
        }
        return blueBlockers;
    }

    std::vector<Player*> GetGreenBlockers() const
    {
        std::vector<Player*> greenBlockers;
        if (Group* group = m_bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                    continue;

                bool hasExhaustion = member->HasAura(SPELL_EXHAUSTION_SERENITY);
                Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
                bool overStack = greenBuff && greenBuff->GetStackAmount() >= 25;
                bool isRogue = member->getClass() == CLASS_ROGUE;
                bool isDpsWarrior = member->getClass() == CLASS_WARRIOR && m_botAI->IsDps(member);
                bool eligibleRogueWarrior = (isRogue || isDpsWarrior) && !hasExhaustion;
                bool isHealer = m_botAI->IsHeal(member);
                bool eligibleHealer = isHealer && !hasExhaustion && !overStack;

                if (eligibleRogueWarrior || eligibleHealer)
                    greenBlockers.push_back(member);
            }
        }
        return greenBlockers;
    }

    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers() const
    {
        Player* redBlocker = nullptr;
        Player* greenBlocker = nullptr;
        Player* blueBlocker = nullptr;

        if (Group* group = m_bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive())
                    continue;

                PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
                if (!memberAI || !memberAI->IsTank(member))
                    continue;
                if (member->HasAura(SPELL_EXHAUSTION_PERSEVERANCE))
                    continue;

                redBlocker = member;
                break;
            }
        }

        std::vector<Player*> greenBlockers = GetGreenBlockers();
        if (!greenBlockers.empty())
            greenBlocker = greenBlockers.front();

        std::vector<Player*> blueBlockers = GetBlueBlockers();
        if (!blueBlockers.empty())
            blueBlocker = blueBlockers.front();

        return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
    }

    std::vector<Unit*> GetAllVoidZones() const
    {
        std::vector<Unit*> voidZones;
        GuidVector npcs = m_botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = m_botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_VOID_ZONE)
                voidZones.push_back(unit);
        }
        return voidZones;
    }

    bool IsSafePosition(float x, float y, float /*z*/, const std::vector<Unit*>& hazards, float hazardRadius) const
    {
        for (Unit* hazard : hazards)
        {
            if (!hazard)
                continue;

            float dx = x - hazard->GetPositionX();
            float dy = y - hazard->GetPositionY();
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < hazardRadius)
                return false;
        }
        return true;
    }

private:
    Player* m_bot;
    PlayerbotAI* m_botAI;
};

bool KarazhanNetherspiteBlockRedBeamAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_PERSEVERANCE, 150.0f);
    Group* group = bot->GetGroup();

    if (!boss || !redPortal || !group)
        return false;

    Player* eligibleTank = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
        if (!memberAI || !memberAI->IsTank(member))
            continue;

        if (member->HasAura(SPELL_EXHAUSTION_PERSEVERANCE))
            continue;

        eligibleTank = member;
        break;
    }

    if (!eligibleTank)
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    Position beamPos = helper.GetPositionOnBeam(boss, redPortal, 18.0f);

    if (bot != eligibleTank)
        return false;

    bot->Yell("I'm moving to block the red beam!", LANG_UNIVERSAL);

    ObjectGuid botGuid = bot->GetGUID();
    uint32 intervalSecs = 5;

    if (g_netherspiteBeamMoveTimes[botGuid] == 0)
    {
        g_netherspiteBeamMoveTimes[botGuid] = time(nullptr);
        g_netherspiteLastMoveSideways[botGuid] = false;
    }

    if (time(nullptr) - g_netherspiteBeamMoveTimes[botGuid] >= intervalSecs)
    {
        g_netherspiteLastMoveSideways[botGuid] = !g_netherspiteLastMoveSideways[botGuid];
        g_netherspiteBeamMoveTimes[botGuid] = time(nullptr);
    }

    if (!g_netherspiteLastMoveSideways[botGuid])
    {
        return MoveTo(bot->GetMapId(), beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float px = redPortal->GetPositionX();
    float py = redPortal->GetPositionY();
    float dx = px - bx;
    float dy = py - by;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0f)
        return false;

    dx /= length;
    dy /= length;
    float perpDx = -dy;
    float perpDy = dx;
    float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
    float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
    float sideZ = beamPos.GetPositionZ();

    return MoveTo(bot->GetMapId(), sideX, sideY, sideZ,
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool KarazhanNetherspiteBlockRedBeamAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* redPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_PERSEVERANCE, 150.0f);

    if (!boss || !redPortal)
        return false;

    static std::map<ObjectGuid, bool> lastBossBanishState;
    ObjectGuid botGuid = bot->GetGUID();
    bool bossIsBanished = IsNetherspiteBanished(boss);

    if (lastBossBanishState[botGuid] != bossIsBanished)
    {
        if (!bossIsBanished)
        {
            g_netherspiteBeamMoveTimes[botGuid] = 0;
            g_netherspiteLastMoveSideways[botGuid] = false;
        }
        lastBossBanishState[botGuid] = bossIsBanished;
    }

    return !bossIsBanished;
}

bool KarazhanNetherspiteBlockBlueBeamAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_DOMINANCE, 150.0f);
    if (!boss || !bluePortal)
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    std::vector<Player*> blueBlockers = helper.GetBlueBlockers();

    ObjectGuid botGuid = bot->GetGUID();
    Player* assignedBlocker = blueBlockers.empty() ? nullptr : blueBlockers.front();
    bool isBlockingNow = bot == assignedBlocker;
    bool wasBlocking = g_netherspiteWasBlockingBlueBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        bot->Yell("I'm leaving the blue beam--next blocker up!", LANG_UNIVERSAL);
        g_netherspiteWasBlockingBlueBeam[botGuid] = false;
    }

    if (!isBlockingNow)
        return false;

    if (!wasBlocking)
        bot->Yell("I'm moving to block the blue beam!", LANG_UNIVERSAL);

    g_netherspiteWasBlockingBlueBeam[botGuid] = true;

    std::vector<Unit*> voidZones = helper.GetAllVoidZones();
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float bz = boss->GetPositionZ();
    float px = bluePortal->GetPositionX();
    float py = bluePortal->GetPositionY();
    float dx = px - bx;
    float dy = py - by;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0f)
        return false;

    dx /= length;
    dy /= length;

    float bestDist = 150.0f;
    Position bestPos;
    bool found = false;

    for (float dist = 18.0f; dist <= 25.0f; dist += 0.5f)
    {
        float candidateX = bx + dx * dist;
        float candidateY = by + dy * dist;
        float candidateZ = bz;

        bool outsideAllVoidZones = true;
        for (Unit* voidZone : voidZones)
        {
            if (!voidZone)
                continue;

            float voidDist = std::sqrt(std::pow(candidateX - voidZone->GetPositionX(), 2.0f) +
                                       std::pow(candidateY - voidZone->GetPositionY(), 2.0f));
            if (voidDist < 4.0f)
            {
                outsideAllVoidZones = false;
                break;
            }
        }

        if (!outsideAllVoidZones)
            continue;

        float distToIdeal = std::fabs(dist - 18.0f);
        if (!found || distToIdeal < bestDist)
        {
            bestDist = distToIdeal;
            bestPos = Position(candidateX, candidateY, candidateZ);
            found = true;
        }
    }

    if (!found)
        return false;

    return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool KarazhanNetherspiteBlockBlueBeamAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* bluePortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_DOMINANCE, 150.0f);

    return boss && bluePortal && !IsNetherspiteBanished(boss);
}

bool KarazhanNetherspiteBlockGreenBeamAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_SERENITY, 150.0f);
    if (!boss || !greenPortal)
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    std::vector<Player*> greenBlockers = helper.GetGreenBlockers();

    ObjectGuid botGuid = bot->GetGUID();
    Player* assignedBlocker = greenBlockers.empty() ? nullptr : greenBlockers.front();
    bool isBlockingNow = bot == assignedBlocker;
    bool wasBlocking = g_netherspiteWasBlockingGreenBeam[botGuid];

    if (wasBlocking && !isBlockingNow)
    {
        bot->Yell("I'm leaving the green beam--next blocker up!", LANG_UNIVERSAL);
        g_netherspiteWasBlockingGreenBeam[botGuid] = false;
    }

    if (!isBlockingNow)
        return false;

    if (!wasBlocking)
        bot->Yell("I'm moving to block the green beam!", LANG_UNIVERSAL);

    g_netherspiteWasBlockingGreenBeam[botGuid] = true;

    std::vector<Unit*> voidZones = helper.GetAllVoidZones();
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float bz = boss->GetPositionZ();
    float px = greenPortal->GetPositionX();
    float py = greenPortal->GetPositionY();
    float dx = px - bx;
    float dy = py - by;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0f)
        return false;

    dx /= length;
    dy /= length;

    float bestDist = 150.0f;
    Position bestPos;
    bool found = false;

    for (float dist = 18.0f; dist <= 25.0f; dist += 0.5f)
    {
        float candidateX = bx + dx * dist;
        float candidateY = by + dy * dist;
        float candidateZ = bz;

        bool outsideAllVoidZones = true;
        for (Unit* voidZone : voidZones)
        {
            if (!voidZone)
                continue;

            float voidDist = std::sqrt(std::pow(candidateX - voidZone->GetPositionX(), 2.0f) +
                                       std::pow(candidateY - voidZone->GetPositionY(), 2.0f));
            if (voidDist < 4.0f)
            {
                outsideAllVoidZones = false;
                break;
            }
        }

        if (!outsideAllVoidZones)
            continue;

        float distToIdeal = std::fabs(dist - 18.0f);
        if (!found || distToIdeal < bestDist)
        {
            bestDist = distToIdeal;
            bestPos = Position(candidateX, candidateY, candidateZ);
            found = true;
        }
    }

    if (!found)
        return false;

    return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool KarazhanNetherspiteBlockGreenBeamAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    Unit* greenPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_SERENITY, 150.0f);

    return boss && greenPortal && !IsNetherspiteBanished(boss);
}

bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss)
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = helper.GetCurrentBeamBlockers();
    if (bot == redBlocker || bot == greenBlocker || bot == blueBlocker)
        return false;

    std::vector<Unit*> voidZones = helper.GetAllVoidZones();

    bool nearVoidZone = std::any_of(voidZones.begin(), voidZones.end(), [&](Unit* vz)
    {
        return vz && bot->GetExactDist2d(vz) < 4.0f;
    });

    struct BeamAvoid
    {
        Unit* portal;
        float minDist;
        float maxDist;
    };

    std::vector<BeamAvoid> beams;
    Unit* redPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_PERSEVERANCE, 150.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_DOMINANCE, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_SERENITY, 150.0f);

    auto addBeam = [&](Unit* portal)
    {
        if (!portal)
            return;

        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float px = portal->GetPositionX();
        float py = portal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = std::sqrt(dx * dx + dy * dy);
        beams.push_back({ portal, 0.0f, length });
    };

    addBeam(redPortal);
    addBeam(bluePortal);
    addBeam(greenPortal);

    bool nearBeam = false;
    for (const auto& beam : beams)
    {
        if (!beam.portal)
            continue;

        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float px = beam.portal->GetPositionX();
        float py = beam.portal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length == 0.0f)
            continue;

        dx /= length;
        dy /= length;
        float botdx = bot->GetPositionX() - bx;
        float botdy = bot->GetPositionY() - by;
        float t = (botdx * dx + botdy * dy);
        float beamX = bx + dx * t;
        float beamY = by + dy * t;
        float distToBeam = std::sqrt(std::pow(bot->GetPositionX() - beamX, 2.0f) +
                                     std::pow(bot->GetPositionY() - beamY, 2.0f));
        if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
        {
            nearBeam = true;
            break;
        }
    }

    if (!nearVoidZone && !nearBeam)
        return false;

    const float minMoveDist = 3.0f;
    const float maxSearchDist = 20.0f;
    const float stepAngle = static_cast<float>(M_PI) / 18.0f;
    const float stepDist = 0.5f;
    float bossZ = boss->GetPositionZ();
    Position bestCandidate;
    float bestDist = 0.0f;
    bool found = false;

    for (float angle = 0.0f; angle < static_cast<float>(2 * M_PI); angle += stepAngle)
    {
        for (float dist = 5.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = bot->GetPositionX() + std::cos(angle) * dist;
            float cy = bot->GetPositionY() + std::sin(angle) * dist;
            float cz = bossZ;

            bool insideVoid = std::any_of(voidZones.begin(), voidZones.end(), [&](Unit* vz)
            {
                return vz && Position(cx, cy, cz).GetExactDist2d(vz) < 4.0f;
            });

            if (insideVoid)
                continue;

            bool tooCloseToBeam = false;
            for (const auto& beam : beams)
            {
                if (!beam.portal)
                    continue;

                float bx = boss->GetPositionX();
                float by = boss->GetPositionY();
                float px = beam.portal->GetPositionX();
                float py = beam.portal->GetPositionY();
                float dx = px - bx;
                float dy = py - by;
                float length = std::sqrt(dx * dx + dy * dy);
                if (length == 0.0f)
                    continue;

                dx /= length;
                dy /= length;
                float botdx = cx - bx;
                float botdy = cy - by;
                float t = (botdx * dx + botdy * dy);
                float beamX = bx + dx * t;
                float beamY = by + dy * t;
                float distToBeam = std::sqrt(std::pow(cx - beamX, 2.0f) + std::pow(cy - beamY, 2.0f));
                if (distToBeam < 5.0f && t > beam.minDist && t < beam.maxDist)
                {
                    tooCloseToBeam = true;
                    break;
                }
            }

            if (tooCloseToBeam)
                continue;

            float moveDist = std::sqrt(std::pow(cx - bot->GetPositionX(), 2.0f) +
                                        std::pow(cy - bot->GetPositionY(), 2.0f));
            if (moveDist < minMoveDist)
                continue;

            if (!found || moveDist < bestDist)
            {
                bestCandidate = Position(cx, cy, cz);
                bestDist = moveDist;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    if (!helper.IsSafePosition(bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
                               bestCandidate.GetPositionZ(), voidZones, 4.0f))
        return false;

    return MoveTo(bot->GetMapId(), bestCandidate.GetPositionX(), bestCandidate.GetPositionY(), bestCandidate.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || IsNetherspiteBanished(boss))
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    auto [redBlocker, greenBlocker, blueBlocker] = helper.GetCurrentBeamBlockers();
    if (bot == redBlocker || bot == blueBlocker || bot == greenBlocker)
        return false;

    return true;
}

bool KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    std::vector<Unit*> voidZones = helper.GetAllVoidZones();

    for (Unit* vz : voidZones)
    {
        if (vz && vz->GetEntry() == NPC_VOID_ZONE && bot->GetExactDist2d(vz) < 4.0f)
            return FleePosition(vz->GetPosition(), 4.0f);
    }

    return false;
}

bool KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !IsNetherspiteBanished(boss))
        return false;

    KarazhanNetherspiteHelper helper(bot, botAI);
    std::vector<Unit*> voidZones = helper.GetAllVoidZones();
    for (Unit* vz : voidZones)
    {
        if (vz && bot->GetExactDist2d(vz) < 4.0f)
            return true;
    }

    return false;
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

    // Position at the side and keep within terrace bounds to avoid falls
    if (distanceToFront < 10.0f || distanceToBack < 10.0f)
    {
        float angle = nightbane->GetOrientation() + (M_PI / 2);
        float x = nightbane->GetPositionX() + cos(angle) * 10.0f;
        float y = nightbane->GetPositionY() + sin(angle) * 10.0f;
        float z = nightbane->GetPositionZ();

        // Clamp within terrace safe circle
        const Position TERRACE_CENTER = { -11162.231f, -1900.329f, 91.476f };
        const float TERRACE_RADIUS = 33.0f;
        float dx = x - TERRACE_CENTER.GetPositionX();
        float dy = y - TERRACE_CENTER.GetPositionY();
        float d = std::sqrt(dx*dx + dy*dy);
        if (d > TERRACE_RADIUS - 1.5f)
        {
            float scale = (TERRACE_RADIUS - 1.5f) / d;
            x = TERRACE_CENTER.GetPositionX() + dx * scale;
            y = TERRACE_CENTER.GetPositionY() + dy * scale;
        }
        
        return MoveTo(bot->GetMapId(), x, y, z, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool NightbanePositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    return nightbane && nightbane->IsInCombat();
}

bool NightbaneCharredEarthAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Prefer dynamic-aoe avoidance using area debuff when present (covers ground patch)
    if (Aura* aura = AI_VALUE(Aura*, "area debuff"))
    {
        if (!aura->IsRemoved() && !aura->IsExpired() && aura->GetType() == DYNOBJ_AURA_TYPE)
        {
            if (DynamicObject* dyn = aura->GetDynobjOwner())
            {
                const SpellInfo* info = aura->GetSpellInfo();
                if (dyn->IsInWorld() && info && info->Id == SPELL_CHARRED_EARTH)
                {
                    float r = dyn->GetRadius();
                    if (r > 0.0f && r <= sPlayerbotAIConfig->maxAoeAvoidRadius && bot->GetDistance(dyn) <= r)
                    {
                        // Flee away from center and clamp within terrace
                        Position fleeFrom = dyn->GetPosition();
                        if (FleePosition(fleeFrom, r))
                        {
                            // After move request, ensure next target stays inside terrace in subsequent ticks
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Fallback: if affected by Charred Earth aura on the bot, step away and clamp within terrace
    if (bot->HasAura(SPELL_CHARRED_EARTH))
    {
        float angle = bot->GetOrientation();
        float x = bot->GetPositionX() + cos(angle) * 12.0f;
        float y = bot->GetPositionY() + sin(angle) * 12.0f;
        float z = bot->GetPositionZ();
        const Position TERRACE_CENTER = { -11162.231f, -1900.329f, 91.476f };
        const float TERRACE_RADIUS = 33.0f;
        float dx = x - TERRACE_CENTER.GetPositionX();
        float dy = y - TERRACE_CENTER.GetPositionY();
        float d = std::sqrt(dx*dx + dy*dy);
        if (d > TERRACE_RADIUS - 1.5f)
        {
            float scale = (TERRACE_RADIUS - 1.5f) / d;
            x = TERRACE_CENTER.GetPositionX() + dx * scale;
            y = TERRACE_CENTER.GetPositionY() + dy * scale;
        }
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
        
    // Useful if Nightbane is casting, or if bot has aura, or if a matching dynamic area is near us
    if (nightbane->FindCurrentSpellBySpellId(SPELL_CHARRED_EARTH) || bot->HasAura(SPELL_CHARRED_EARTH))
        return true;
    if (Aura* aura = AI_VALUE(Aura*, "area debuff"))
    {
        const SpellInfo* info = aura->GetSpellInfo();
        if (info && info->Id == SPELL_CHARRED_EARTH)
            return true;
    }
    return false;
}

bool NightbaneAirPhaseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;

    // Throttle movement to avoid spam
    static std::map<ObjectGuid, uint32> s_lastAirMove;
    uint32 now = getMSTime();
    uint32& last = s_lastAirMove[bot->GetGUID()];
    if (last && now - last < 800)
        return false;

    // Nightbane air phase: boss is not in melee range
    if (!nightbane->IsWithinMeleeRange(bot))
    {
        // Define the Master's Terrace safe center and a conservative radius to stay on the platform
        static const Position TERRACE_CENTER = { -11162.231f, -1900.329f, 91.476f };
        static constexpr float TERRACE_RADIUS = 33.0f; // keep well within terrace edges
        static constexpr float SLOT_RING = 14.0f;      // spread ring for players during air phase

        // Pick a stable slot around center based on GUID to avoid stacking
        float baseAngle = float((bot->GetGUID().GetCounter() % 360)) * float(M_PI) / 180.0f;
        // Slightly vary for roles
        if (botAI->IsHeal(bot))
            baseAngle += 0.35f;

        float targetX = TERRACE_CENTER.GetPositionX() + std::cos(baseAngle) * SLOT_RING;
        float targetY = TERRACE_CENTER.GetPositionY() + std::sin(baseAngle) * SLOT_RING;
        float targetZ = TERRACE_CENTER.GetPositionZ(); // anchor to terrace Z to avoid falls/teleporting

        // Clamp to terrace circle if somehow outside
        float dx = targetX - TERRACE_CENTER.GetPositionX();
        float dy = targetY - TERRACE_CENTER.GetPositionY();
        float d  = std::sqrt(dx*dx + dy*dy);
        if (d > TERRACE_RADIUS - 2.0f)
        {
            float scale = (TERRACE_RADIUS - 2.0f) / d;
            targetX = TERRACE_CENTER.GetPositionX() + dx * scale;
            targetY = TERRACE_CENTER.GetPositionY() + dy * scale;
        }

        // Keep within 35y of Nightbane to avoid Fireball Barrage targeting
        float nbDist = bot->GetExactDist2d(nightbane);
        if (nbDist > 35.0f)
        {
            // Move a bit toward boss, but stay clamped to terrace ring
            float toBoss = std::atan2(nightbane->GetPositionY() - TERRACE_CENTER.GetPositionY(),
                                      nightbane->GetPositionX() - TERRACE_CENTER.GetPositionX());
            targetX = TERRACE_CENTER.GetPositionX() + std::cos(toBoss) * std::min(SLOT_RING + 2.0f, TERRACE_RADIUS - 3.0f);
            targetY = TERRACE_CENTER.GetPositionY() + std::sin(toBoss) * std::min(SLOT_RING + 2.0f, TERRACE_RADIUS - 3.0f);
        }

        // If we are already close enough to our slot, do nothing
        if (bot->GetExactDist2d(targetX, targetY) < 3.0f)
            return false;

        last = now;
        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
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
            // Ignore skeletons that are off the terrace to prevent trash pulls
            const Position TERRACE_CENTER = { -11162.231f, -1900.329f, 91.476f };
            const float TERRACE_RADIUS = 36.0f;
            if (unit->GetExactDist2d(TERRACE_CENTER) > TERRACE_RADIUS)
                continue;
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
// Helper to detect active chess environment even if GAME_IN_SESSION aura is not present on the bot
static bool IsChessEnvironmentActive(Player* bot, PlayerbotAI* botAI)
{
    if (!bot)
        return false;
    // Aura present on player or any nearby group member
    if (bot->HasAura(SPELL_GAME_IN_SESSION))
        return true;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member->GetMapId() != bot->GetMapId())
                continue;
            if (member->IsAlive() && member->GetDistance(bot) < 120.0f && member->HasAura(SPELL_GAME_IN_SESSION))
                return true;
        }
    }

    if (botAI && AnyNearbyChessPiece(botAI, [](Creature* piece)
    {
        uint32 entry = piece->GetEntry();
        bool isHumanPiece = entry == NPC_HUMAN_FOOTMAN || entry == NPC_HUMAN_CONJURER || entry == NPC_HUMAN_CLERIC || entry == NPC_HUMAN_CHARGER || entry == NPC_CHESS_KING_LLANE;
        bool isOrcPiece = entry == NPC_ORC_GRUNT || entry == NPC_ORC_WARLOCK || entry == NPC_ORC_NECROLYTE || entry == NPC_ORC_WOLF || entry == NPC_WARCHIEF_BLACKHAND;
        return (isHumanPiece || isOrcPiece) && !piece->GetCharmerGUID().IsEmpty();
    }))
    {
        return true;
    }

    return false;
}
bool ChessEventMoveAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if bot is controlling a chess piece (vehicle) or charmed piece
    Unit* vehicle = bot->GetVehicleBase();
    Creature* controlledPiece = nullptr;
    // Shared reservation map for piece selection
    struct Reservation { ObjectGuid bot; uint32 ts; };
    static std::map<ObjectGuid, Reservation> s_pieceReservations; // piece -> (bot, time)
    if (!vehicle)
    {
        // Try to possess a chess piece only if event is active around us
        if (!IsChessEnvironmentActive(bot, botAI))
            return false;

        // Skip if bot recently left a piece (server aura blocks immediate re-entry)
        if (bot->HasAura(30529)) // SPELL_RECENTLY_INGAME
            return false;

        // Choose a side based on bot team and build role-prioritized piece order
        bool isHorde = (bot->GetTeamId() == TEAM_HORDE);
        auto preferList = [&]() -> std::vector<uint32>
        {
            bool isTank = botAI->IsTank(bot);
            bool isHeal = botAI->IsHeal(bot);
            bool isMelee = botAI->IsMelee(bot) && !isTank;
            // Treat ranged as default when not tank/heal/melee
            if (isHorde)
            {
                // Reserve the King (Warchief Blackhand) for humans: do not include it in bot preferences
                if (isTank)  return {NPC_ORC_WOLF, NPC_ORC_GRUNT, NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE};
                if (isHeal)  return {NPC_ORC_NECROLYTE, NPC_ORC_WARLOCK, NPC_ORC_WOLF, NPC_ORC_GRUNT};
                if (isMelee) return {NPC_ORC_WOLF, NPC_ORC_GRUNT, NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE};
                return {NPC_ORC_WARLOCK, NPC_ORC_NECROLYTE, NPC_ORC_WOLF, NPC_ORC_GRUNT};
            }
            else
            {
                // Reserve the King (King Llane) for humans: do not include it in bot preferences
                if (isTank)  return {NPC_HUMAN_CHARGER, NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC};
                if (isHeal)  return {NPC_HUMAN_CLERIC, NPC_HUMAN_CONJURER, NPC_HUMAN_CHARGER, NPC_HUMAN_FOOTMAN};
                if (isMelee) return {NPC_HUMAN_CHARGER, NPC_HUMAN_FOOTMAN, NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC};
                return {NPC_HUMAN_CONJURER, NPC_HUMAN_CLERIC, NPC_HUMAN_CHARGER, NPC_HUMAN_FOOTMAN};
            }
        }();

        // Build list of free pieces on our side, grouped by entry
        std::map<uint32, std::vector<Creature*>> freeByEntry;
        ForEachNearbyNpc(botAI, 120.0f, [&](Unit* unit)
        {
            Creature* c = unit->ToCreature();
            if (!c)
                return;

            uint32 entry = c->GetEntry();
            bool ours = isHorde ? (entry == NPC_ORC_GRUNT || entry == NPC_ORC_WARLOCK || entry == NPC_ORC_NECROLYTE || entry == NPC_ORC_WOLF || entry == NPC_WARCHIEF_BLACKHAND)
                                 : (entry == NPC_HUMAN_FOOTMAN || entry == NPC_HUMAN_CONJURER || entry == NPC_HUMAN_CLERIC || entry == NPC_HUMAN_CHARGER || entry == NPC_CHESS_KING_LLANE);

            if (!ours || !c->IsAlive() || c->GetCharmer() || c->GetVehicle())
                return;

            freeByEntry[entry].push_back(c);
        });

        // Filter duplicates and reserve logic
        const uint32 now = getMSTime();
        const uint32 RESERVE_TIMEOUT = 5000; // 5s

        auto reservedByOther = [&](Creature* c) -> bool {
            auto it = s_pieceReservations.find(c->GetGUID());
            if (it == s_pieceReservations.end()) return false;
            const Reservation& r = it->second;
            if (now - r.ts > RESERVE_TIMEOUT) return false; // stale
            return r.bot != bot->GetGUID();
        };

        // From the preferred order, find a free, non-reserved piece
        Creature* piece = nullptr;
        for (uint32 entry : preferList)
        {
            auto it = freeByEntry.find(entry);
            if (it == freeByEntry.end() || it->second.empty())
                continue;
            // Stable distribution among same entry
            auto& vec = it->second;
            size_t idx = bot->GetGUID().GetCounter() % vec.size();
            // Scan up to N candidates to avoid a reserved one
            for (size_t k = 0; k < vec.size(); ++k)
            {
                Creature* candidate = vec[(idx + k) % vec.size()];
                // Explicitly skip kings as an extra safety, even if they slip into lists
                if (candidate->GetEntry() == NPC_CHESS_KING_LLANE || candidate->GetEntry() == NPC_WARCHIEF_BLACKHAND)
                    continue;
                if (!reservedByOther(candidate))
                {
                    piece = candidate;
                    break;
                }
            }
            if (piece)
                break;
        }
        if (!piece)
            return false;

        // Reserve the chosen piece
        s_pieceReservations[piece->GetGUID()] = { bot->GetGUID(), now };
        

        // Move into gossip range
        if (bot->GetDistance(piece) > 4.5f)
        {
            bot->GetMotionMaster()->MovePoint(0, piece->GetPositionX(), piece->GetPositionY(), piece->GetPositionZ());
            
            return true;
        }

        // Open gossip and select "Control <piece>"
        {
            // Ensure the piece currently allows gossip (not in warmup/moving)
            if (!piece->HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
                return false;
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
                uint32 giIndex = it->first;
                GossipMenuItem const* gi = menu.GetItem(giIndex);
                if (!gi)
                    continue;
                if (gi->Message.find("Control ") == 0)
                {
                    selectIndex = static_cast<int32>(giIndex);
                    break;
                }
                if (selectIndex == -1)
                    selectIndex = static_cast<int32>(giIndex);
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

        // We own a piece now; clear any stale reservations made by this bot
        for (auto it = s_pieceReservations.begin(); it != s_pieceReservations.end(); )
        {
            if (it->second.bot == bot->GetGUID()) it = s_pieceReservations.erase(it); else ++it;
        }
    }
    else
    {
        // Already in a vehicle: clear any reservations for this bot
        for (auto it = s_pieceReservations.begin(); it != s_pieceReservations.end(); )
        {
            if (it->second.bot == bot->GetGUID()) it = s_pieceReservations.erase(it); else ++it;
        }
    }
        
    Creature* pieceCreature = nullptr;
    if (vehicle)
        pieceCreature = vehicle->ToCreature();
    if (!pieceCreature)
        pieceCreature = controlledPiece;
    if (!pieceCreature)
        return false;

    uint32 pieceEntry = pieceCreature->GetEntry();
    bool isRanged = (pieceEntry == NPC_HUMAN_CONJURER || pieceEntry == NPC_ORC_WARLOCK ||
                     pieceEntry == NPC_HUMAN_CLERIC || pieceEntry == NPC_ORC_NECROLYTE);
    bool isKing = (pieceEntry == NPC_CHESS_KING_LLANE || pieceEntry == NPC_WARCHIEF_BLACKHAND);
    
    Creature* target = SelectChessEnemy(pieceCreature, 60.0f);
    if (!target)
        return false;
    
    // Movement strategy using chess triggers (cast SPELL_MOVE_GENERIC on best trigger)
    float desiredDistance = isRanged ? 18.0f : (isKing ? 10.0f : 6.0f);
    float currentDistance = pieceCreature->GetDistance(target);
    uint32 now = getMSTime();
    uint32 lastAbility = g_chess_lastAbilityTime[pieceCreature->GetGUID()];
    bool stale = (lastAbility && now - lastAbility > 3500);

    if (fabs(currentDistance - desiredDistance) > 3.5f || stale)
    {
        if (pieceCreature->HasAura(KZ_SPELL_MOVE_COOLDOWN))
            return false;

        uint32& last = g_chess_lastMoveTime[pieceCreature->GetGUID()];
        if (last && now - last < 900)
            return false;

        std::list<Creature*> triggers;
        pieceCreature->GetCreatureListWithEntryInGrid(triggers, KZ_NPC_CHESS_MOVE_TRIGGER, 25.0f);

        Creature* bestTrigger = nullptr;
        float bestScore = std::numeric_limits<float>::max();
        for (Creature* trig : triggers)
        {
            if (!trig)
                continue;

            float dPiece = pieceCreature->GetDistance(trig);
            if (dPiece > 25.0f)
                continue;

            float dEnemy = trig->GetDistance(target);
            float score = dEnemy + dPiece * 0.3f;
            if (score < bestScore)
            {
                bestScore = score;
                bestTrigger = trig;
            }
        }

        if (!bestTrigger)
            bestTrigger = pieceCreature->FindNearestCreature(KZ_NPC_CHESS_MOVE_TRIGGER, 25.0f, true);

        if (bestTrigger)
        {
            pieceCreature->CastSpell(bestTrigger, KZ_SPELL_MOVE_GENERIC, false);
            g_chess_lastMoveTime[pieceCreature->GetGUID()] = now;
            return true;
        }
    }

    pieceCreature->SetFacingTo(pieceCreature->GetAngle(target));

    return false;
}

bool ChessEventMoveAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    // Useful if already controlling a piece or the chess environment is active around us
    if (bot->GetVehicleBase())
        return true;
    if (IsChessEnvironmentActive(bot, botAI))
        return true;
    return false;
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

    // Safety throttles to avoid spam; allow casts even if generic casting flags flicker
    if (piece->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;
    uint32 now = getMSTime();
    uint32& last = g_chess_lastAbilityTime[piece->GetGUID()];
    if (last && now - last < 900) // keep cadence snappy
        return false;

    uint32 pieceEntry = piece->GetEntry();

    auto selectEnemyInRange = [&](float range) -> Creature*
    {
        Creature* enemy = SelectChessEnemy(piece, range);
        if (enemy && piece->GetDistance(enemy) <= range)
            return enemy;
        return nullptr;
    };

    auto selectAllyForHeal = [&](float range, uint32 minMissing) -> Creature*
    {
        Creature* ally = SelectChessAllyForHeal(piece, range, minMissing);
        if (ally && piece->GetDistance(ally) <= range)
            return ally;
        return nullptr;
    };

    switch (pieceEntry)
    {
        case NPC_HUMAN_FOOTMAN: // Pawn A
        {
            if (Creature* e = selectEnemyInRange(12.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
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
            if (Creature* e = selectEnemyInRange(10.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(piece, 37498, true); // Stomp
                piece->CastSpell(e, 37453, false);    // Smash
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_HUMAN_CONJURER: // Queen A
        {
            if (Creature* e = selectEnemyInRange(25.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(e, 37462, false); // Elemental Blast
                piece->CastSpell(e, 37465, false); // Rain of Fire
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_HUMAN_CLERIC: // Bishop A
        {
            if (Creature* ally = selectAllyForHeal(25.0f, 5000))
            {
                piece->CastSpell(ally, 37455, false); // Healing
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            if (Creature* e = selectEnemyInRange(20.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(e, 37459, false); // Holy Lance
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_CHESS_KING_LLANE: // King A
        {
            piece->CastSpell(piece, 37471, true); // Heroism
            if (Creature* e = selectEnemyInRange(10.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(piece, 37474, true); // Sweep
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_GRUNT: // Pawn H
        {
            if (Creature* e = selectEnemyInRange(12.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
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
            if (Creature* e = selectEnemyInRange(10.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(piece, 37454, true); // Bite
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_WARLOCK: // Queen H
        {
            if (Creature* e = selectEnemyInRange(25.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(e, 37463, false); // Fireball
                piece->CastSpell(e, 37469, false); // Poison Cloud
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_ORC_NECROLYTE: // Bishop H
        {
            if (Creature* ally = selectAllyForHeal(25.0f, 5000))
            {
                piece->CastSpell(ally, 37456, false); // Shadow Mend
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            if (Creature* e = selectEnemyInRange(20.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
                piece->CastSpell(e, 37461, false); // Shadow Spear
                g_chess_lastAbilityTime[piece->GetGUID()] = now;
                return true;
            }
            return false;
        }
        case NPC_WARCHIEF_BLACKHAND: // King H
        {
            piece->CastSpell(piece, 37472, true); // Bloodlust
            if (Creature* e = selectEnemyInRange(10.0f))
            {
                piece->SetFacingTo(piece->GetAngle(e));
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
    
    bool interrupted = false;

    ForEachNearbyNpc(botAI, 30.0f, [&](Unit* target)
    {
        if (interrupted || !target->IsHostileTo(bot))
            return;

        if (!target->HasUnitState(UNIT_STATE_CASTING))
            return;

        bool shouldInterrupt = false;
        uint32 npcId = target->GetEntry();

        if (npcId == NPC_SHADE_OF_ARAN && IsCastingSpell(target, SPELL_ARCANE_MISSILES))
            shouldInterrupt = true;

        if ((npcId == NPC_BARONESS_DOROTHEA || npcId == NPC_LORD_ROBIN) && target->HasUnitState(UNIT_STATE_CASTING))
            shouldInterrupt = true;

        if (npcId == NPC_DOROTHEE && target->HasUnitState(UNIT_STATE_CASTING))
            shouldInterrupt = true;

        if (!shouldInterrupt)
            return;

        if (currentTime - rotation.lastInterruptTime <= 2000 && rotation.lastInterrupter != bot->GetGUID())
            return;

        bool spellInterrupted = false;
        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
                spellInterrupted = botAI->CastSpell(6552, target); // Pummel
                break;
            case CLASS_ROGUE:
                spellInterrupted = botAI->CastSpell(1766, target); // Kick
                break;
            case CLASS_MAGE:
                spellInterrupted = botAI->CastSpell(2139, target); // Counterspell
                break;
            case CLASS_SHAMAN:
                spellInterrupted = botAI->CastSpell(8042, target); // Earth Shock
                break;
            case CLASS_DEATH_KNIGHT:
                spellInterrupted = botAI->CastSpell(47528, target); // Mind Freeze
                break;
            default:
                break;
        }

        if (spellInterrupted)
        {
            rotation.lastInterruptTime = currentTime;
            rotation.lastInterrupter = bot->GetGUID();
            interrupted = true;
        }
    });

    return interrupted;
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
