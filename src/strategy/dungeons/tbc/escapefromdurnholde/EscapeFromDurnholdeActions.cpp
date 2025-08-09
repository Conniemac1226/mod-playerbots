#include "EscapeFromDurnholdeActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "DungeonStrategyUtils.h"
#include <cmath>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_F
#define M_PI_F 3.14159265f
#endif

// Per-bot state management for whirlwind avoidance
static std::map<ObjectGuid, uint32> g_drake_lastMoveTime;
static std::map<ObjectGuid, bool> g_drake_inSafePosition;

// Per-bot state management for Epoch Hunter positioning
static std::map<ObjectGuid, uint32> g_epoch_lastMoveTime;
static std::map<ObjectGuid, bool> g_epoch_inSafePosition;

bool HealThrallAction::Execute(Event event)
{
    // Find Thrall within range
    Creature* thrall = bot->FindNearestCreature(NPC_THRALL, 100.0f);
    if (!thrall || !thrall->IsAlive())
        return false;

    // Execute class-specific healing logic using direct CastSpell like Valithria
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            // Difficulty-appropriate TBC Druid healing spells
            uint32 rejuvenationId = TBC_MODE(bot, 26981u, 26982u);  // Rank 12 (Normal) / Rank 13 (Heroic)
            uint32 regrowthId = TBC_MODE(bot, 26979u, 26980u);      // Rank 9 (Normal) / Rank 10 (Heroic)  
            uint32 lifebloomId = 33763u;                            // Rank 1 (same for both difficulties)
            
            // Apply Rejuvenation if missing
            if (!thrall->HasAura(rejuvenationId, bot->GetGUID()))
                return botAI->CastSpell(rejuvenationId, thrall);
            
            // Apply Lifebloom if missing
            if (!thrall->HasAura(lifebloomId, bot->GetGUID()))
                return botAI->CastSpell(lifebloomId, thrall);
                
            // Cast Regrowth
            return botAI->CastSpell(regrowthId, thrall);
        }
        case CLASS_PRIEST:
        {
            // Difficulty-appropriate TBC Priest healing spells
            uint32 renewId = TBC_MODE(bot, 25221u, 25315u);        // Rank 11 (Normal) / Rank 12 (Heroic)
            uint32 greaterHealId = TBC_MODE(bot, 2060u, 25314u);   // Rank 6 (Normal) / Rank 7 (Heroic)
            uint32 flashHealId = TBC_MODE(bot, 25235u, 25233u);    // Rank 8 (Normal) / Rank 9 (Heroic)
            
            // Cast Greater Heal if Renew is active, otherwise apply Renew
            return thrall->HasAura(renewId, bot->GetGUID()) 
                ? botAI->CastSpell(greaterHealId, thrall)
                : botAI->CastSpell(renewId, thrall);
        }
        case CLASS_PALADIN:
        {
            // Difficulty-appropriate TBC Paladin healing spells
            uint32 flashOfLightId = TBC_MODE(bot, 19941u, 27137u);  // Rank 6 (Normal) / Rank 7 (Heroic)
            uint32 holyLightId = TBC_MODE(bot, 25292u, 27136u);     // Rank 10 (Normal) / Rank 11 (Heroic)
            
            // Use Holy Light for more healing
            return botAI->CastSpell(holyLightId, thrall);
        }
        case CLASS_SHAMAN:
        {
            // Difficulty-appropriate TBC Shaman healing spells
            uint32 earthShieldId = TBC_MODE(bot, 32593u, 32594u);   // Rank 2 (Normal) / Rank 3 (Heroic)
            uint32 healingWaveId = TBC_MODE(bot, 25396u, 25357u);   // Rank 11 (Normal) / Rank 12 (Heroic)  
            uint32 chainHealId = TBC_MODE(bot, 25422u, 25423u);     // Rank 4 (Normal) / Rank 5 (Heroic)
            
            // Apply Earth Shield if missing, otherwise cast Healing Wave
            return !thrall->HasAura(earthShieldId, bot->GetGUID())
                ? botAI->CastSpell(earthShieldId, thrall)
                : botAI->CastSpell(healingWaveId, thrall);
        }
        // Death Knights and Warlocks cannot heal friendly NPCs
        case CLASS_DEATH_KNIGHT:
        case CLASS_WARLOCK:
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_HUNTER:
        case CLASS_MAGE:
            return false;
    }

    return false;
}

bool ThrallEmergencyHealAction::Execute(Event event)
{
    // Find Thrall within range
    Creature* thrall = bot->FindNearestCreature(NPC_THRALL, 100.0f);
    if (!thrall || !thrall->IsAlive())
        return false;

    // Execute class-specific emergency healing logic using direct CastSpell
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            uint32 swiftmendId = 26981u;                                    // Rank 1 (same for both difficulties)
            uint32 regrowthId = TBC_MODE(bot, 26979u, 26980u);              // Rank 9 (Normal) / Rank 10 (Heroic)
            uint32 rejuvNormalId = TBC_MODE(bot, 26981u, 26982u);           // For HoT checking
            
            // Try Swiftmend if HoTs are present, otherwise Regrowth
            if (thrall->HasAura(rejuvNormalId, bot->GetGUID()) || thrall->HasAura(regrowthId, bot->GetGUID()))
                return botAI->CastSpell(swiftmendId, thrall);
            return botAI->CastSpell(regrowthId, thrall);
        }
        case CLASS_PRIEST:
        {
            uint32 flashHealId = TBC_MODE(bot, 25235u, 25233u);    // Rank 8 (Normal) / Rank 9 (Heroic)
            return botAI->CastSpell(flashHealId, thrall);
        }
        case CLASS_PALADIN:
        {
            uint32 flashOfLightId = TBC_MODE(bot, 19941u, 27137u); // Rank 6 (Normal) / Rank 7 (Heroic)
            return botAI->CastSpell(flashOfLightId, thrall);
        }
        case CLASS_SHAMAN:
        {
            uint32 lesserHealingWaveId = TBC_MODE(bot, 25420u, 25420u); // Rank 7 (same for both difficulties)
            return botAI->CastSpell(lesserHealingWaveId, thrall);
        }
        default:
            return false;
    }
}

bool EfdAvoidWhirlwindAction::Execute(Event event)
{
    // Validate bot exists
    if (!bot)
        return false;
    
    Unit* drake = AI_VALUE2(Unit*, "find target", "lieutenant drake");
    if (!drake || !drake->IsAlive())
        return false;
    
    // Per-bot state management
    uint32 currentTime = getMSTime();
    ObjectGuid botGuid = bot->GetGUID();
    
    // Check if Drake is casting or has whirlwind active
    bool isWhirlwinding = false;
    if (drake->HasUnitState(UNIT_STATE_CASTING) && drake->FindCurrentSpellBySpellId(SPELL_DRAKE_WHIRLWIND))
        isWhirlwinding = true;
    if (drake->HasAura(SPELL_DRAKE_WHIRLWIND))
        isWhirlwinding = true;
    
    // Check if we're already in a safe position during this whirlwind
    if (g_drake_inSafePosition[botGuid] && isWhirlwinding)
    {
        // Check if this is a new whirlwind phase (10+ seconds since last move)
        if ((currentTime - g_drake_lastMoveTime[botGuid]) > 10000)
        {
            g_drake_inSafePosition[botGuid] = false;
            g_drake_lastMoveTime[botGuid] = 0;
        }
        else
        {
            return false; // Already safe, don't move again
        }
    }
    
    // Reset state if whirlwind ended
    if (!isWhirlwinding)
    {
        g_drake_inSafePosition[botGuid] = false;
        g_drake_lastMoveTime[botGuid] = 0;
        return false;
    }
    
    // Calculate unique safe position for this bot
    float baseAngle = (botGuid.GetCounter() % 8) * (M_PI / 4.0f); // Distribute bots in 8 directions
    float angle = baseAngle + frand(-0.2f, 0.2f); // Add small random variation
    float distance = 15.0f;
    
    float targetX = drake->GetPositionX() + cos(angle) * distance;
    float targetY = drake->GetPositionY() + sin(angle) * distance;
    float targetZ = drake->GetPositionZ();
    
    // Move to safe position using proper movement API
    bool result = MoveTo(bot->GetMapId(), targetX, targetY, targetZ, 
                        false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    
    if (result)
    {
        g_drake_lastMoveTime[botGuid] = currentTime;
        g_drake_inSafePosition[botGuid] = true;
    }
    
    return result;
}

bool EfdAvoidWhirlwindAction::isUseful()
{
    Unit* drake = AI_VALUE2(Unit*, "find target", "lieutenant drake");
    if (!drake || !drake->IsAlive())
        return false;
    
    // Check if Drake is CASTING whirlwind or has the aura
    bool isWhirlwinding = false;
    
    // Check for active cast
    if (drake->HasUnitState(UNIT_STATE_CASTING) && drake->FindCurrentSpellBySpellId(SPELL_DRAKE_WHIRLWIND))
        isWhirlwinding = true;
    
    // Check for aura    
    if (drake->HasAura(SPELL_DRAKE_WHIRLWIND))
        isWhirlwinding = true;
        
    if (!isWhirlwinding)
        return false;
    
    // Check if we're already safe
    ObjectGuid botGuid = bot->GetGUID();
    if (g_drake_inSafePosition[botGuid])
        return false;
    
    // Need to move if within whirlwind range (8 yards is typical warrior whirlwind range)
    return bot->GetDistance(drake) < 10.0f;
}

bool DispelHammerOfJusticeAction::Execute(Event event)
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_PALADIN)
        return false;
    
    if (bot->getClass() == CLASS_PRIEST)
        return AI_VALUE2(bool, "cast spell", "dispel magic");
    else if (bot->getClass() == CLASS_PALADIN)
        return AI_VALUE2(bool, "cast spell", "cleanse");
    
    return false;
}

bool DispelHammerOfJusticeAction::isUseful()
{
    return bot->HasAura(SPELL_SKARLOC_HAMMER);
}

bool EpochHunterPositionAction::Execute(Event event)
{
    // Validate bot exists
    if (!bot)
        return false;
    
    Unit* epoch = AI_VALUE2(Unit*, "find target", "epoch hunter");
    if (!epoch || !epoch->IsAlive())
        return false;
    
    // Per-bot state management
    uint32 currentTime = getMSTime();
    ObjectGuid botGuid = bot->GetGUID();
    
    // Check if we're already positioned
    if (g_epoch_inSafePosition[botGuid])
    {
        // Check if we need to reposition (boss turned or new phase)
        if ((currentTime - g_epoch_lastMoveTime[botGuid]) > 5000)
        {
            // Re-evaluate position every 5 seconds
            g_epoch_inSafePosition[botGuid] = false;
            g_epoch_lastMoveTime[botGuid] = 0;
        }
        else if (!epoch->HasInArc(M_PI / 3.0f, bot))
        {
            return false; // Still safe
        }
    }
    
    // Calculate unique side position for this bot
    bool leftSide = (botGuid.GetCounter() % 2) == 0;
    float sideAngle = epoch->GetOrientation() + (leftSide ? M_PI_F / 2.0f : -M_PI_F / 2.0f);
    float distance = 8.0f + frand(-1.0f, 1.0f); // Vary distance slightly
    
    float targetX = epoch->GetPositionX() + cos(sideAngle) * distance;
    float targetY = epoch->GetPositionY() + sin(sideAngle) * distance;
    float targetZ = epoch->GetPositionZ();
    
    // Move to safe position using proper movement API
    bool result = MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                        false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    
    if (result)
    {
        g_epoch_lastMoveTime[botGuid] = currentTime;
        g_epoch_inSafePosition[botGuid] = true;
    }
    
    return result;
}

bool EpochHunterPositionAction::isUseful()
{
    // Validate bot exists
    if (!bot)
        return false;
    
    Unit* epoch = AI_VALUE2(Unit*, "find target", "epoch hunter");
    if (!epoch || !epoch->IsAlive())
        return false;
    
    // Check if we're already safe
    ObjectGuid botGuid = bot->GetGUID();
    if (g_epoch_inSafePosition[botGuid] && !epoch->HasInArc(M_PI / 3.0f, bot))
        return false;
    
    // Check if bot is within Sand Breath range and in front arc
    if (bot->GetDistance2d(epoch) < 15.0f)
    {
        // Use researched HasInArc API from Unit.h:1845 for proper cone detection
        return epoch->HasInArc(M_PI / 3.0f, bot);
    }
    
    return false;
}

bool CancelMagicDisruptionAction::Execute(Event event)
{
    if (bot->IsNonMeleeSpellCast(false))
    {
        bot->InterruptNonMeleeSpells(false);
        return true;
    }
    
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
        return bot->Attack(target, true);
    
    return false;
}

bool CancelMagicDisruptionAction::isUseful()
{
    if (!bot->HasAura(SPELL_EPOCH_MAGIC_DISRUPTION))
        return false;
    
    return bot->IsNonMeleeSpellCast(false);
}

bool EfdReturnPositionAction::Execute(Event event)
{
    if (!bot)
        return false;
    
    // Reset safe position states when returning
    ObjectGuid botGuid = bot->GetGUID();
    g_drake_inSafePosition[botGuid] = false;
    g_epoch_inSafePosition[botGuid] = false;
    
    // Return to current target if we have one
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive())
    {
        float distance = bot->GetDistance(target);
        
        // Move to melee range for melee classes, ranged for others
        float optimalDistance = bot->IsWithinMeleeRange(target) ? 5.0f : 20.0f;
        
        if (distance > optimalDistance)
        {
            return MoveTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    // Otherwise follow tank or master
    Unit* followTarget = AI_VALUE(Unit*, "tank target");
    if (!followTarget)
        followTarget = AI_VALUE(Unit*, "master target");
    
    if (followTarget)
    {
        return MoveTo(followTarget->GetMapId(), followTarget->GetPositionX(), followTarget->GetPositionY(), followTarget->GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool EfdReturnPositionAction::isUseful()
{
    if (!bot)
        return false;
    
    ObjectGuid botGuid = bot->GetGUID();
    
    // Return if we're marked as safe but the danger has passed
    if (g_drake_inSafePosition[botGuid])
    {
        Unit* drake = AI_VALUE2(Unit*, "find target", "lieutenant drake");
        if (!drake || !drake->IsAlive() || !drake->HasAura(SPELL_DRAKE_WHIRLWIND))
            return true;
    }
    
    if (g_epoch_inSafePosition[botGuid])
    {
        Unit* epoch = AI_VALUE2(Unit*, "find target", "epoch hunter");
        if (!epoch || !epoch->IsAlive())
            return true;
    }
    
    return false;
}