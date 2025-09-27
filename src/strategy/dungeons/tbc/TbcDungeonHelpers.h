#ifndef _PLAYERBOT_TBC_DUNGEON_HELPERS_H
#define _PLAYERBOT_TBC_DUNGEON_HELPERS_H

#include "PlayerbotAI.h"
#include "Value.h"

#include <unordered_set>

namespace TbcDungeon
{
    template <typename Predicate>
    inline void ForEachNearbyNpc(PlayerbotAI* botAI, Player* bot, float maxDistance, Predicate&& predicate)
    {
        if (!botAI || !bot)
            return;

        std::unordered_set<ObjectGuid> seen;

        auto visitCache = [&](char const* cacheName)
        {
            if (Value<GuidVector>* value = botAI->GetAiObjectContext()->GetValue<GuidVector>(cacheName))
            {
                for (ObjectGuid const& guid : value->Get())
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
            }
        };

        visitCache("nearest hostile npcs");
        visitCache("nearest npcs");
    }

    inline Unit* FindClosestNpcByEntry(PlayerbotAI* botAI, Player* bot, uint32 entry, float maxDistance)
    {
        Unit* closest = nullptr;
        float closestDist = maxDistance;

        ForEachNearbyNpc(botAI, bot, maxDistance, [&](Unit* unit)
        {
            if (unit->GetEntry() != entry)
                return;

            float distance = bot->GetDistance(unit);
            if (distance < closestDist)
            {
                closestDist = distance;
                closest = unit;
            }
        });

        return closest;
    }
}

#endif // _PLAYERBOT_TBC_DUNGEON_HELPERS_H
