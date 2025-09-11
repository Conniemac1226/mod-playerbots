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
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (flame->GetVictim() == bot)
        {
            // Being chased - only allow instant spells
            std::string spellName = spellAction->getSpell();
            uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
            
            if (spellId > 0)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (spellInfo && spellInfo->CalcCastTime() == 0)
                    return 1.0f; // Allow instant cast
            }
            
            return 0.0f; // Block cast time spells
        }
        // Note flames in 18y for AoE suppression
        if (!flameNearby && bot->GetExactDist2d(flame) < 18.0f)
            flameNearby = true;
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
                // Also block long casts while flames are near (DPS safety)
                if (spellInfo->CalcCastTime() > 1500 && !botAI->IsHeal(bot))
                    return 0.0f;
            }
        }
    }

    return 1.0f;
}
