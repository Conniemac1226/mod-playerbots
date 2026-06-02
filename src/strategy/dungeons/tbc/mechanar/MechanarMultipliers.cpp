#include "MechanarMultipliers.h"
#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include <string>

float MechanarMultiplier::GetValue(Action* action)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    // Block all spells if standing in fire trail / inferno damage
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA) || bot->HasAura(SPELL_INFERNO_DAMAGE))
        return 0.0f;
        
    // Check if bot is being targeted by Raging Flames for cast time restriction
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    bool flameNearby = false;
    Unit* chasingFlame = nullptr;
    float chasingFlameDistance = 1000.0f;
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        float flameDistance = bot->GetExactDist2d(flame);
        if (flame->GetVictim() == bot)
        {
            chasingFlame = flame;
            chasingFlameDistance = flameDistance;
        }
        // Note flames in 18y for AoE suppression
        if (!flameNearby && flameDistance < 18.0f)
            flameNearby = true;
    }

    if (chasingFlame)
    {
        std::string spellName = spellAction->getSpell();
        uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
        SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        bool const safeGap = chasingFlameDistance > (bot->GetMap()->IsHeroic() ? 26.0f : 23.0f) && !flameNearby;

        if (!spellInfo)
            return safeGap ? 1.0f : 0.0f;

        uint32 castTime = spellInfo->CalcCastTime();
        if (castTime == 0)
            return 1.0f;
        if (safeGap && (castTime <= 1500 || botAI->IsHeal(bot)))
            return 1.0f;

        return 0.0f;
    }

    // Suppress AoE/ground-targeted spells near flames (prevents stepping into danger and wasted casts)
    if (flameNearby)
    {
        std::string spellName = spellAction->getSpell();
        uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
        if (spellId)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (spellInfo)
            {
                // Heuristic: block ground-target and wide AoE when kiting
                if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
                    return 0.0f;
                // Block long casts for all roles when flames are near
                if (spellInfo->CalcCastTime() > 1200)
                {
                    // Allow healers to push long heals only if relatively safe
                    if (botAI->IsHeal(bot))
                    {
                        // If any flame within 14y, block
                        const GuidVector npcs2 = AI_VALUE(GuidVector, "nearest hostile npcs");
                        for (auto& g : npcs2)
                        {
                            Unit* f = botAI->GetUnit(g);
                            if (f && f->IsAlive() && f->GetEntry() == NPC_RAGING_FLAMES && bot->GetExactDist2d(f) < 14.0f)
                                return 0.0f;
                        }
                        // Otherwise allow the heal
                        return 1.0f;
                    }
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}
