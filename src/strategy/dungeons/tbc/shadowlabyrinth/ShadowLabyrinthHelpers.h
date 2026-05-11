#ifndef _PLAYERBOT_SHADOWLABYRINTHHELPERS_H
#define _PLAYERBOT_SHADOWLABYRINTHHELPERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"

#include <unordered_map>

const uint32 SPELL_CORROSIVE_ACID = 33551;
const uint32 SL_SPELL_FEAR = 33547;

const uint32 SPELL_INCITE_CHAOS = 33676;
const uint32 SPELL_INCITE_CHAOS_B = 33684;
const uint32 SL_SPELL_CHARGE = 33709;
const uint32 SL_SPELL_WAR_STOMP = 33707;

const uint32 SPELL_RAIN_OF_FIRE = 33617;
const uint32 SPELL_DRAW_SHADOWS = 33563;
const uint32 SPELL_SHADOWBOLT_VOLLEY = 33841;
const uint32 SPELL_BANISH = 38791;

const uint32 SPELL_SONIC_BOOM_CAST = 33923;
const uint32 SPELL_SONIC_BOOM_EFFECT = 38795;
const uint32 SPELL_MURMURS_TOUCH = 33711;
const uint32 SPELL_MURMURS_TOUCH_HEROIC = 38794;
const uint32 SPELL_RESONANCE = 33657;
const uint32 SL_SPELL_MAGNETIC_PULL = 33689;
const uint32 SPELL_THUNDERING_STORM = 39365;

const uint32 NPC_VOID_TRAVELER = 19226;
const uint32 NPC_GRANDMASTER_VORPIL = 18732;
const uint32 NPC_MURMUR = 18708;

namespace ShadowLabyrinth
{
struct VorpilCache
{
    uint32 lastScanMs = 0;
    ObjectGuid cachedTravelerGuid = ObjectGuid::Empty;
    uint32 lastMoveMs = 0;
    Position lastMovePos = {};
};

struct MurmurCache
{
    uint32 sonicBoomDangerUntilMs = 0;
    uint32 sonicBoomReturnUntilMs = 0;
    uint32 touchDangerUntilMs = 0;
    uint32 touchReturnUntilMs = 0;
    ObjectGuid touchedPlayerGuid = ObjectGuid::Empty;
    uint32 lastMoveMs = 0;
    Position lastMovePos = {};
};

bool IsGrandmasterVorpil(Unit const* unit);
bool IsMurmur(Unit const* unit);
bool IsVoidTraveler(Unit const* unit);

VorpilCache& GetVorpilCache(ObjectGuid const& botGuid);
MurmurCache& GetMurmurCache(ObjectGuid const& botGuid);

bool IsMurmurCastingSonicBoom(Unit const* boss);
bool HasMurmursTouch(Player const* player);

Unit* FindNearestVoidTravelerCached(PlayerbotAI* botAI, Player* bot, Unit* boss, float maxDistance = 80.0f);
Player* FindTouchedPlayerCached(PlayerbotAI* botAI, Player* bot, float maxDistance = 18.0f);

bool ShouldIssueMovement(uint32& lastMoveMs, Position& lastMovePos, Position const& destination,
    uint32 now, uint32 cooldownMs, float minimumDelta);

Position GetVorpilCenter();
Position GetVorpilSafeSpreadPosition(Player* bot, Unit* boss, Player const* nearestAlly);
Position GetMurmurSafeMovePosition(Player* bot, Unit* boss, Unit const* threatSource, float desiredDistance);

} // namespace ShadowLabyrinth

#endif
