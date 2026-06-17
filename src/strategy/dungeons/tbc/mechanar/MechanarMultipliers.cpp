#include "MechanarMultipliers.h"
#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include <algorithm>
#include <string>

float MechanarMultiplier::GetValue(Action* action)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    // Check if bot is being targeted by Raging Flames for cast time restriction
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    Unit* chasingFlame = nullptr;
    float chasingFlameDistance = 1000.0f;
    float nearestFlameDistance = 1000.0f;
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        float flameDistance = bot->GetExactDist2d(flame);
        nearestFlameDistance = std::min(nearestFlameDistance, flameDistance);
        if (flame->GetVictim() == bot)
        {
            chasingFlame = flame;
            chasingFlameDistance = flameDistance;
        }
    }

    float const tooCloseDistance = bot->GetMap()->IsHeroic() ? 14.0f : 12.0f;
    if (nearestFlameDistance < tooCloseDistance)
        return 0.0f;

    std::string spellName = spellAction->getSpell();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;

    if (chasingFlame)
    {
        bool const safeGap = chasingFlameDistance > (bot->GetMap()->IsHeroic() ? 22.0f : 19.0f) &&
                             nearestFlameDistance > (bot->GetMap()->IsHeroic() ? 18.0f : 16.0f);

        if (!spellInfo)
            return safeGap ? 1.0f : 0.0f;

        uint32 castTime = spellInfo->CalcCastTime();
        if (castTime == 0)
            return 1.0f;
        if (safeGap && castTime <= 1500)
            return 1.0f;
        if (safeGap && botAI->IsHeal(bot) && castTime <= 2500)
            return 1.0f;

        return 0.0f;
    }

    // Keep nearby flames from pulling ground-targeted spells into danger, but do not
    // freeze the rest of the kit when a flame is only passing by.
    if (nearestFlameDistance < 18.0f)
    {
        if (spellInfo)
        {
            if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
                return 0.0f;

            if (spellInfo->CalcCastTime() > 2500 && nearestFlameDistance < 14.0f)
                return 0.0f;
        }
    }

    return 1.0f;
}
