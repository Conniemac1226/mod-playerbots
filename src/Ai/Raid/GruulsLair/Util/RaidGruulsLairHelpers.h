#ifndef RAID_GRUULSLAIRHELPERS_H
#define RAID_GRUULSLAIRHELPERS_H

#include <ctime>
#include <unordered_map>

#include "Event.h"
#include "PlayerbotAI.h"

namespace GruulsLairHelpers
{
    enum GruulsLairSpells
    {
        // High King Maulgar
        SPELL_WHIRLWIND     = 33238,

        // Krosh Firehand
        SPELL_SPELL_SHIELD  = 33054,

        // Blindeye the Seer
        SPELL_HEAL          = 33144,
        SPELL_PRAYER_OF_HEALING = 33152,

        // Hunter
        SPELL_MISDIRECTION  = 35079,

        // Gruul the Dragonkiller
        SPELL_GROUND_SLAM_1 = 33525,
        SPELL_GROUND_SLAM_2 = 39187,
    };

    enum GruulsLairNPCs
    {
        NPC_WILD_FEL_STALKER = 18847,
    };

    constexpr uint32 GRUULS_LAIR_MAP_ID = 565;

    bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
    bool IsAnyCouncilBossInCombat(PlayerbotAI* botAI);
    bool IsHighKingMaulgarPullAllowed(PlayerbotAI* botAI, Player* bot);
    bool IsGruulTheDragonkillerPullAllowed(PlayerbotAI* botAI, Player* bot);
    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
    bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
    bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);
    bool TryGetMaulgarCouncilPosition(PlayerbotAI* botAI, Player* bot, Unit* focusTarget,
                                      Position& outPos);
    Unit* FindHighKingMaulgar(PlayerbotAI* botAI, Player* bot);
    Unit* FindGruulTheDragonkiller(PlayerbotAI* botAI, Player* bot);
    bool IsGruulsLairAutoPullReady(PlayerbotAI* botAI, Player* bot);
    Unit* SelectGruulsLairTrashPullTarget(PlayerbotAI* botAI, Player* bot);
    void MarkGruulsLairBossPullReady(PlayerbotAI* botAI, Player* bot);
    extern std::unordered_map<uint32, time_t> gruulDpsWaitTimer;

    extern const Position MAULGAR_TANK_POSITION;
    extern const Position OLM_TANK_POSITION;
    extern const Position BLINDEYE_TANK_POSITION;
    extern const Position KROSH_TANK_POSITION;
    extern const Position MAULGAR_ROOM_CENTER;
    extern const Position MAULGAR_HEALER_POSITION;
    extern const Position MAULGAR_SUPPORT_POSITION;
    extern const Position BLINDEYE_DPS_POSITION;
    extern const Position OLM_DPS_POSITION;
    extern const Position KIGGLER_TANK_POSITION;
    extern const Position GRUUL_TANK_POSITION;
}

#endif
