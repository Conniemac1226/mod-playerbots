#include "ShadowLabyrinthHelpers.h"

#include "Group.h"
#include "ObjectAccessor.h"
#include "SpellAuras.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kVorpilCenterX = -253.548f;
constexpr float kVorpilCenterY = -263.646f;
constexpr float kVorpilCenterZ = 17.0864f;

std::unordered_map<ObjectGuid, ShadowLabyrinth::VorpilCache> g_vorpilCache;
std::unordered_map<ObjectGuid, ShadowLabyrinth::MurmurCache> g_murmurCache;

Position MakePosition(float x, float y, float z)
{
    Position pos;
    pos.Relocate(x, y, z);
    return pos;
}

Player* GetTouchedGroupMember(PlayerbotAI* botAI, Player* bot, float maxDistance, uint32& touchExpireMs, ObjectGuid& touchedGuid)
{
    Player* touched = nullptr;
    float bestDistance = maxDistance;

    if (!botAI || !bot)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    auto considerPlayer = [&](Player* member)
    {
        if (!member || member == bot || !member->IsAlive())
            return;

        if (!ShadowLabyrinth::HasMurmursTouch(member))
            return;

        float const distance = bot->GetDistance(member);
        if (distance > maxDistance || distance >= bestDistance)
            return;

        Aura* touch = member->GetAura(SPELL_MURMURS_TOUCH);
        if (!touch)
            touch = member->GetAura(SPELL_MURMURS_TOUCH_HEROIC);
        if (touch)
        {
            int32 duration = std::max(0, touch->GetDuration());
            touchExpireMs = std::max(touchExpireMs, getMSTime() + static_cast<uint32>(duration));
        }

        touched = member;
        bestDistance = distance;
        touchedGuid = member->GetGUID();
    };

    considerPlayer(bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        considerPlayer(ref->GetSource());

    return touched;
}
}

bool ShadowLabyrinth::IsGrandmasterVorpil(Unit const* unit)
{
    return unit && unit->GetEntry() == NPC_GRANDMASTER_VORPIL;
}

bool ShadowLabyrinth::IsMurmur(Unit const* unit)
{
    return unit && unit->GetEntry() == NPC_MURMUR;
}

bool ShadowLabyrinth::IsVoidTraveler(Unit const* unit)
{
    return unit && unit->GetEntry() == NPC_VOID_TRAVELER;
}

ShadowLabyrinth::VorpilCache& ShadowLabyrinth::GetVorpilCache(ObjectGuid const& botGuid)
{
    return g_vorpilCache[botGuid];
}

ShadowLabyrinth::MurmurCache& ShadowLabyrinth::GetMurmurCache(ObjectGuid const& botGuid)
{
    return g_murmurCache[botGuid];
}

bool ShadowLabyrinth::IsMurmurCastingSonicBoom(Unit const* boss)
{
    if (!boss)
        return false;

    if (boss->FindCurrentSpellBySpellId(SPELL_SONIC_BOOM_CAST) || boss->HasAura(SPELL_SONIC_BOOM_CAST))
        return true;

    if (!boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell const* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell)
        currentSpell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

    return currentSpell && (currentSpell->m_spellInfo->Id == SPELL_SONIC_BOOM_CAST || currentSpell->m_spellInfo->Id == SPELL_SONIC_BOOM_EFFECT);
}

bool ShadowLabyrinth::HasMurmursTouch(Player const* player)
{
    return player && (player->HasAura(SPELL_MURMURS_TOUCH) || player->HasAura(SPELL_MURMURS_TOUCH_HEROIC));
}

Unit* ShadowLabyrinth::FindNearestVoidTravelerCached(PlayerbotAI* botAI, Player* bot, Unit* boss, float maxDistance)
{
    if (!botAI || !bot || !boss || !IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
        return nullptr;

    VorpilCache& cache = GetVorpilCache(bot->GetGUID());
    uint32 const now = getMSTime();

    if (cache.cachedTravelerGuid)
    {
        if (Unit* cached = botAI->GetUnit(cache.cachedTravelerGuid))
        {
            if (cached->IsAlive() && cached->IsInWorld() && cached->GetMapId() == bot->GetMapId() &&
                bot->GetDistance(cached) <= maxDistance)
            {
                return cached;
            }
        }
    }

    if (cache.lastScanMs && now - cache.lastScanMs < 350U)
        return nullptr;

    cache.lastScanMs = now;

    Unit* bestTraveler = nullptr;
    float bestDistanceToBoss = maxDistance;

    auto considerUnit = [&](Unit* unit)
    {
        if (!unit || !unit->IsAlive() || !IsVoidTraveler(unit))
            return;

        float const distanceToBoss = boss->GetDistance(unit);
        if (distanceToBoss > maxDistance || distanceToBoss >= bestDistanceToBoss)
            return;

        bestTraveler = unit;
        bestDistanceToBoss = distanceToBoss;
    };

    if (AiObjectContext* context = botAI->GetAiObjectContext())
    {
        if (auto* npcs = context->GetValue<GuidVector>("nearest hostile npcs"))
        {
            for (ObjectGuid const& guid : npcs->Get())
                considerUnit(botAI->GetUnit(guid));
        }

        if (!bestTraveler)
        {
            if (auto* npcs = context->GetValue<GuidVector>("nearest npcs"))
            {
                for (ObjectGuid const& guid : npcs->Get())
                    considerUnit(botAI->GetUnit(guid));
            }
        }
    }

    if (!bestTraveler)
        TbcDungeon::ForEachNearbyNpc(botAI, bot, maxDistance, considerUnit);

    if (bestTraveler)
    {
        cache.cachedTravelerGuid = bestTraveler->GetGUID();
    }

    return bestTraveler;
}

Player* ShadowLabyrinth::FindTouchedPlayerCached(PlayerbotAI* botAI, Player* bot, float maxDistance)
{
    if (!botAI || !bot)
        return nullptr;

    MurmurCache& cache = GetMurmurCache(bot->GetGUID());
    uint32 const now = getMSTime();
    uint32 touchExpireMs = cache.touchDangerUntilMs;
    ObjectGuid touchedGuid = ObjectGuid::Empty;

    Player* touched = GetTouchedGroupMember(botAI, bot, maxDistance, touchExpireMs, touchedGuid);
    if (touched)
    {
        cache.touchedPlayerGuid = touchedGuid;
        cache.touchDangerUntilMs = touchExpireMs;
        cache.touchReturnUntilMs = std::max(cache.touchReturnUntilMs, cache.touchDangerUntilMs + 1500U);
        return touched;
    }

    cache.touchedPlayerGuid = ObjectGuid::Empty;
    if (touchExpireMs > now)
        cache.touchDangerUntilMs = touchExpireMs;

    return nullptr;
}

bool ShadowLabyrinth::ShouldIssueMovement(uint32& lastMoveMs, Position& lastMovePos, Position const& destination,
    uint32 now, uint32 cooldownMs, float minimumDelta)
{
    if (lastMoveMs && now >= lastMoveMs && now - lastMoveMs < cooldownMs)
        return false;

    if (lastMovePos.IsPositionValid())
    {
        float const dx = lastMovePos.GetPositionX() - destination.GetPositionX();
        float const dy = lastMovePos.GetPositionY() - destination.GetPositionY();
        float const dz = lastMovePos.GetPositionZ() - destination.GetPositionZ();
        float const distSq = dx * dx + dy * dy + dz * dz;
        if (distSq < minimumDelta * minimumDelta)
            return false;
    }

    lastMoveMs = now;
    lastMovePos.Relocate(destination);
    return true;
}

Position ShadowLabyrinth::GetVorpilCenter()
{
    return MakePosition(kVorpilCenterX, kVorpilCenterY, kVorpilCenterZ);
}

Position ShadowLabyrinth::GetVorpilSafeSpreadPosition(Player* bot, Unit* boss, Player const* nearestAlly)
{
    Position bossPos = boss ? boss->GetPosition() : GetVorpilCenter();
    float const baseDistance = (bot && bot->GetLevel() >= 70) ? 18.0f : 15.0f;

    if (nearestAlly)
    {
        float angle = nearestAlly->GetAngle(bot) + kPi;
        return MakePosition(bot->GetPositionX() + std::cos(angle) * baseDistance,
            bot->GetPositionY() + std::sin(angle) * baseDistance,
            bossPos.GetPositionZ());
    }

    float angle = bot ? bot->GetAngle(&bossPos) : 0.0f;
    if (!std::isfinite(angle))
        angle = 0.0f;

    return MakePosition(bossPos.GetPositionX() + std::cos(angle) * 20.0f,
        bossPos.GetPositionY() + std::sin(angle) * 20.0f,
        bossPos.GetPositionZ());
}

Position ShadowLabyrinth::GetMurmurSafeMovePosition(Player* bot, Unit* boss, Unit const* threatSource, float desiredDistance)
{
    if (!bot || !boss)
        return MakePosition(bot ? bot->GetPositionX() : 0.0f, bot ? bot->GetPositionY() : 0.0f, bot ? bot->GetPositionZ() : 0.0f);

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();
    float const bossZ = boss->GetPositionZ();

    if (threatSource)
    {
        float const angle = threatSource->GetAngle(bot) + kPi;
        return MakePosition(bot->GetPositionX() + std::cos(angle) * desiredDistance,
            bot->GetPositionY() + std::sin(angle) * desiredDistance,
            bossZ);
    }

    float const angle = bot->GetAngle(boss) + kPi;
    return MakePosition(bossX + std::cos(angle) * desiredDistance,
        bossY + std::sin(angle) * desiredDistance,
        bossZ);
}
