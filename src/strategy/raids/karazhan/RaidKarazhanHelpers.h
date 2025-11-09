#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include "AiObject.h"
#include "Playerbots.h"
#include "Position.h"

enum KarazhanSpells
{
    // Maiden of Virtue
    SPELL_REPENTANCE                 = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD     = 30756,

    // Shade of Aran
    SPELL_FLAME_WREATH               = 30004,
    SPELL_AURA_FLAME_WREATH          = 29946,
    SPELL_ARCANE_EXPLOSION           = 29973,
    SPELL_WARLOCK_BANISH             = 18647, // Rank 2

    // Netherspite
    SPELL_GREEN_BEAM_DEBUFF          = 30422,
    SPELL_BLUE_BEAM_DEBUFF           = 30423,
    SPELL_NETHER_EXHAUSTION_RED      = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN    = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE     = 38639,
    SPELL_NETHERSPITE_BANISHED       = 39833,

    // Prince Malchezaar
    SPELL_ENFEEBLE                   = 30843,

    // Chess Event
    SPELL_GAME_IN_SESSION            = 39331,
    SPELL_CONTROL_PIECE              = 30019,
    SPELL_RECENTLY_INGAME            = 30529,
};

enum KarazhanNpcs
{
    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Terestian Illhoof
    NPC_KILREK                       = 17229,
    NPC_DEMON_CHAINS                 = 17248,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    NPC_VOID_ZONE                    = 16697,
    NPC_RED_PORTAL                   = 17369,
    NPC_BLUE_PORTAL                  = 17368,
    NPC_GREEN_PORTAL                 = 17367,

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL         = 17646,

    // Chess Event
    NPC_ECHO_OF_MEDIVH               = 16816,

    // Alliance pieces (chess piece type = job name)
    NPC_KING_A                       = 21684, // King Llane
    NPC_QUEEN_A                      = 21683, // Human Conjurer
    NPC_BISHOP_A                     = 21682, // Human Cleric
    NPC_KNIGHT_A                     = 21664, // Human Charger
    NPC_ROOK_A                       = 21160, // Human Conjured Water Elemental
    NPC_PAWN_A                       = 17211, // Human Footman

    // Horde pieces
    NPC_KING_H                       = 21752, // Warchief Blackhand
    NPC_QUEEN_H                      = 21750, // Orc Warlock
    NPC_BISHOP_H                     = 21747, // Orc Necrolyte
    NPC_KNIGHT_H                     = 21748, // Orc Wolf
    NPC_ROOK_H                       = 21726, // Orc Grunt (Rook)
    NPC_PAWN_H                       = 17469, // Orc Grunt (Pawn)

    // Legacy naming for backwards compatibility
    NPC_CHESS_KING_LLANE             = 21684,
    NPC_WARCHIEF_BLACKHAND           = 21752,
    NPC_HUMAN_FOOTMAN                = 17211,
    NPC_ORC_GRUNT                    = 17469,
    NPC_HUMAN_CHARGER                = 21664,
    NPC_ORC_WOLF                     = 21748,
    NPC_HUMAN_CONJURER               = 21683,
    NPC_ORC_WARLOCK                  = 21750,
    NPC_HUMAN_CLERIC                 = 21682,
    NPC_ORC_NECROLYTE                = 21747,

    NPC_CHESS_MOVE_TRIGGER           = 22519,
};

// Chess helper constants (from core script boss_chess_event.cpp / karazhan.h)
const uint32 KZ_SPELL_MOVE_GENERIC = 30012;
const uint32 KZ_SPELL_MOVE_COOLDOWN = 30543;
const uint32 KZ_SPELL_CHANGE_FACING = 30284;

extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION;
extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8];
extern const Position KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION;
extern const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4];
extern const Position KARAZHAN_THE_CURATOR_BOSS_POSITION;

class RaidKarazhanHelpers : public AiObject
{
public:
    explicit RaidKarazhanHelpers(PlayerbotAI* botAI) : AiObject(botAI) {}

    void MarkTargetWithSkull(Unit* /*target*/);
    Unit* GetFirstAliveUnit(const std::vector<Unit*>& /*units*/);
    Unit* GetFirstAliveUnitByEntry(uint32 /*entry*/);
    Unit* GetNearestPlayerInRadius(float /*radius*/ = 5.0f);
    bool IsFlameWreathActive();
    Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss);
    std::vector<Player*> GetRedBlockers();
    std::vector<Player*> GetBlueBlockers();
    std::vector<Player*> GetGreenBlockers();
    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers();
    std::vector<Unit*> GetAllVoidZones();
    bool IsSafePosition (float x, float y, float z,
         const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetSpawnedInfernals() const;
    bool IsStraightPathSafe(const Position& start, const Position& target,
         const std::vector<Unit*>& hazards, float hazardRadius, float stepSize);
};

#endif
