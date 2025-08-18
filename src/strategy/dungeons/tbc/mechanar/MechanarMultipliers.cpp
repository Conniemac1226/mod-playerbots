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

    // Smart spell blocking - allow instant casts while kiting
    if (CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action))
    {
        // Block all spells if standing in fire trail
        if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA))
        {
            return 0.0f;
        }
        
        // Check if bot is being targeted by Raging Flames
        bool beingChased = false;
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        
        for (auto& npc : npcs)
        {
            if (!botAI)
                continue;
                
            Unit* flame = botAI->GetUnit(npc);
            if (!flame || !flame->IsAlive() || !flame->IsInWorld())
                continue;
                
            if (flame->GetEntry() != NPC_RAGING_FLAMES)
                continue;
                
            Unit* victim = flame->GetVictim();
            if (victim && victim == bot)
            {
                beingChased = true;
                break;
            }
        }
        
        if (beingChased)
        {
            // Use AzerothCore API to check spell cast time
            std::string spellName = spellAction->getSpell();
            uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
            
            if (spellId > 0)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (spellInfo)
                {
                    // Check if spell is instant cast
                    uint32 castTime = spellInfo->CalcCastTime();
                    if (castTime == 0)
                    {
                        // Allow instant cast spells while being chased
                        return 1.0f;
                    }
                }
            }
            
            // Block cast time spells while being chased
            return 0.0f;
        }
    }

    return 1.0f;
}