#include "BlackTempleActions.h"
#include "AiObjectContext.h"
#include "GameObject.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "Unit.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Value.h"
#include "Item.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include <ctime>

#define SPELL_IMPALING_SPINE 39837
#define SPELL_TIDAL_SHIELD 39872
#define ITEM_NAJENTUS_SPINE 32408
#define NPC_NAJENTUS 22887

static std::map<ObjectGuid, uint32> g_najentus_lastAvoidTime;
static std::map<ObjectGuid, bool> g_najentus_hasSpine;

bool NajentusAvoidImpaledTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    uint32 currentTime = getMSTime();
    
    // Don't move too frequently
    if (g_najentus_lastAvoidTime[botGuid] > 0 && 
        (currentTime - g_najentus_lastAvoidTime[botGuid]) < 3000)
        return false;

    // Check for impaled allies nearby
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        // Check if they have Impaling Spine debuff
        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            float distance = bot->GetDistance(member);
            // Stay at least 10 yards away from impaled targets
            if (distance < 10.0f)
            {
                // Calculate escape position
                float angle = bot->GetAngle(member) + M_PI; // Move opposite direction
                float moveDistance = 12.0f - distance; // Move to safe distance
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                if (MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true, 
                          MovementPriority::MOVEMENT_FORCED))
                {
                    g_najentus_lastAvoidTime[botGuid] = currentTime;
                    return true;
                }
            }
        }
    }

    return false;
}

bool NajentusSpinePickupAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if we already have a spine
    if (bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
    {
        g_najentus_hasSpine[botGuid] = true;
        return false;
    }

    g_najentus_hasSpine[botGuid] = false;

    // Only ranged DPS should pick up spines
    if (botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false;

    // Check if we're melee
    if (botAI->IsMelee(bot))
        return false;

    // Look for spine game objects
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    GameObject* nearestSpine = nullptr;
    float nearestDistance = 40.0f; // Max pickup range

    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // Check if it's a spine object (would need correct entry ID)
        if (go->GetGoType() == GAMEOBJECT_TYPE_GENERIC)
        {
            float distance = bot->GetDistance(go);
            if (distance < nearestDistance)
            {
                nearestSpine = go;
                nearestDistance = distance;
            }
        }
    }

    if (nearestSpine && nearestDistance > 5.0f)
    {
        return MoveTo(nearestSpine->GetMapId(), 
                     nearestSpine->GetPositionX(),
                     nearestSpine->GetPositionY(), 
                     nearestSpine->GetPositionZ(), 
                     false, false, false, true,
                     MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool NajentusThrowSpineAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss has Tidal Shield
    if (!boss->HasAura(SPELL_TIDAL_SHIELD))
        return false;

    // Check if we have a spine
    if (!bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
        return false;

    // Check range (20-30 yards optimal for spine throw)
    float distance = bot->GetDistance(boss);
    if (distance < 20.0f || distance > 35.0f)
    {
        float targetDistance = 25.0f;
        float angle = bot->GetAngle(boss);
        float moveDistance = targetDistance - distance;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_COMBAT);
    }

    // Use the spine item
    Item* spine = bot->GetItemByEntry(ITEM_NAJENTUS_SPINE);
    if (spine)
    {
        bot->SetFacingToObject(boss);
        
        // Use item on boss
        bot->CastSpell(boss, spine->GetTemplate()->Spells[0].SpellId, true, spine);
        
        ObjectGuid botGuid = bot->GetGUID();
        g_najentus_hasSpine[botGuid] = false;
        
        return true;
    }

    return false;
}

bool NajentusTidalBurstPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if Tidal Shield is up or about to break
    if (!boss->HasAura(SPELL_TIDAL_SHIELD))
        return false;

    // Healers and ranged should spread out for Tidal Burst
    if (!botAI->IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    // Check distance to other players
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    bool tooClose = false;
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f) // Need at least 8 yards spread
        {
            tooClose = true;
            break;
        }
    }

    if (tooClose)
    {
        // Find a better position with more spread
        float angle = bot->GetOrientation() + (float(rand()) / RAND_MAX) * M_PI * 2;
        float moveDistance = 10.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        // Make sure we don't move too far from boss
        Position bossPos = boss->GetPosition();
        float newDistanceToBoss = sqrt(pow(newX - bossPos.GetPositionX(), 2) + 
                                      pow(newY - bossPos.GetPositionY(), 2));
        
        if (newDistanceToBoss < 40.0f) // Stay within reasonable range
        {
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

// Supremus
#define SPELL_SNARE_SELF 41922
#define SPELL_MOLTEN_FLAME 40980
#define SPELL_VOLCANIC_ERUPTION 40276
#define NPC_SUPREMUS 22898
#define NPC_SUPREMUS_VOLCANO 23085

static std::map<ObjectGuid, uint32> g_supremus_lastKiteTime;
static std::map<ObjectGuid, bool> g_supremus_isFixated;
static std::map<ObjectGuid, uint32> g_supremus_phase; // 0 = unknown, 1 = tank phase, 2 = kite phase

bool SupremusPhaseCheckAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if boss has snare (kite phase)
    bool hasSnare = boss->HasAura(SPELL_SNARE_SELF);
    uint32 currentPhase = hasSnare ? 2 : 1;
    
    if (g_supremus_phase[botGuid] != currentPhase)
    {
        g_supremus_phase[botGuid] = currentPhase;
        g_supremus_isFixated[botGuid] = false;
        
        // Phase changed - adjust strategy
        if (currentPhase == 2)
        {
            // Kite phase - check if we're fixated
            Unit* target = boss->GetVictim();
            if (target == bot)
            {
                g_supremus_isFixated[botGuid] = true;
            }
        }
    }

    return true;
}

bool SupremusKiteAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Only kite if we're fixated during kite phase
    if (g_supremus_phase[botGuid] != 2 || !g_supremus_isFixated[botGuid])
        return false;

    // Check if boss is targeting us
    if (boss->GetVictim() != bot)
    {
        g_supremus_isFixated[botGuid] = false;
        return false;
    }

    uint32 currentTime = getMSTime();
    
    // Don't kite too frequently
    if (g_supremus_lastKiteTime[botGuid] > 0 && 
        (currentTime - g_supremus_lastKiteTime[botGuid]) < 2000)
        return false;

    float distance = bot->GetDistance(boss);
    
    // Keep 30-40 yard distance during kite phase
    if (distance < 30.0f)
    {
        // Run away from boss
        float angle = bot->GetAngle(boss) + M_PI;
        float moveDistance = 35.0f - distance;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        // Stay within bounds (from boss script)
        if (newX < 565.0f) newX = 565.0f;
        if (newX > 865.0f) newX = 865.0f;
        if (newY < 545.0f) newY = 545.0f;
        if (newY > 1000.0f) newY = 1000.0f;
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        if (MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED))
        {
            g_supremus_lastKiteTime[botGuid] = currentTime;
            return true;
        }
    }

    return false;
}

bool SupremusAvoidVolcanoAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    // Look for volcano NPCs
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        // Check if it's a volcano
        if (unit->GetEntry() == NPC_SUPREMUS_VOLCANO)
        {
            float distance = bot->GetDistance(unit);
            
            // Volcanoes have 15 yard danger radius
            if (distance < 15.0f)
            {
                // Move away from volcano
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = 20.0f - distance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool SupremusAvoidMoltenFlameAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Look for molten flame trails (ground effects)
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // Molten flame creates fire patches on ground
        float distance = bot->GetDistance(go);
        if (distance < 8.0f)
        {
            // Move away from flame
            float angle = bot->GetAngle(go) + M_PI;
            float moveDistance = 10.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool SupremusHatefulStrikePositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Only position during tank phase
    if (g_supremus_phase[botGuid] != 1)
        return false;

    // Only tanks need special positioning for hateful strike
    if (!botAI->IsTank(bot))
        return false;

    // Check if we're in melee range
    float distance = bot->GetDistance(boss);
    if (distance > 5.0f)
    {
        // Move to melee range
        float angle = bot->GetAngle(boss);
        float moveDistance = distance - 3.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_COMBAT);
    }

    // Face the boss
    if (!bot->HasInArc(M_PI / 6, boss))
    {
        bot->SetFacingToObject(boss);
    }

    return false;
}

// Shade of Akama
#define NPC_SHADE_OF_AKAMA 22841
#define NPC_AKAMA_SHADE 23191
#define NPC_ASHTONGUE_CHANNELER 23421
#define NPC_ASHTONGUE_SORCERER 23215
#define NPC_ASHTONGUE_DEFENDER 23216
#define NPC_ASHTONGUE_ELEMENTAL 23523
#define NPC_ASHTONGUE_ROGUE 23318
#define NPC_ASHTONGUE_SPIRITBIND 23524

static std::map<ObjectGuid, uint32> g_shade_lastChannelerCheck;
static std::map<ObjectGuid, uint32> g_shade_phase; // 1 = channelers, 2 = shade released

bool ShadeOfAkamaChannelerAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check for channelers
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* channeler = nullptr;
    float nearestDistance = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ASHTONGUE_CHANNELER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < nearestDistance)
            {
                channeler = unit;
                nearestDistance = distance;
            }
        }
    }

    if (channeler)
    {
        // Switch to channeler if not already targeting
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget != channeler)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(channeler);
            bot->SetTarget(channeler->GetGUID());
        }

        // Move to channeler if needed
        if (nearestDistance > (botAI->IsRanged(bot) ? 25.0f : 5.0f))
        {
            float angle = bot->GetAngle(channeler);
            float targetDistance = botAI->IsRanged(bot) ? 20.0f : 3.0f;
            float moveDistance = nearestDistance - targetDistance;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }

        g_shade_phase[botGuid] = 1;
        return true;
    }

    // No channelers left, shade should be released
    g_shade_phase[botGuid] = 2;
    return false;
}

bool ShadeOfAkamaAddsAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Priority order for adds
    static const uint32 addPriority[] = {
        NPC_ASHTONGUE_SORCERER,    // Highest priority (casters)
        NPC_ASHTONGUE_SPIRITBIND,  // Healers
        NPC_ASHTONGUE_ELEMENTAL,   // Elementals
        NPC_ASHTONGUE_ROGUE,       // Rogues
        NPC_ASHTONGUE_DEFENDER     // Tanks (lowest)
    };

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* priorityTarget = nullptr;
    float nearestDistance = 50.0f;
    int highestPriority = -1;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if it's an add we care about
        for (int i = 0; i < 5; i++)
        {
            if (unit->GetEntry() == addPriority[i])
            {
                if (i <= highestPriority || highestPriority == -1)
                {
                    float distance = bot->GetDistance(unit);
                    if (i < highestPriority || (i == highestPriority && distance < nearestDistance))
                    {
                        priorityTarget = unit;
                        nearestDistance = distance;
                        highestPriority = i;
                    }
                }
                break;
            }
        }
    }

    if (priorityTarget)
    {
        // Switch to priority add
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget != priorityTarget)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(priorityTarget);
            bot->SetTarget(priorityTarget->GetGUID());
        }
        return true;
    }

    return false;
}

bool ShadeOfAkamaProtectAkamaAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Find Akama
    Unit* akama = AI_VALUE2(Unit*, "find target", "akama");
    if (!akama || !akama->IsAlive())
        return false;

    // Check Akama's health
    float akamaHealthPct = akama->GetHealthPct();
    if (akamaHealthPct > 30.0f)
        return false;

    // Healers should prioritize healing Akama
    if (botAI->IsHeal(bot))
    {
        // Move closer to Akama if needed for healing
        float distance = bot->GetDistance(akama);
        if (distance > 30.0f)
        {
            float angle = bot->GetAngle(akama);
            float moveDistance = distance - 25.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // DPS should kill adds threatening Akama
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* nearestThreat = nullptr;
    float nearestDistance = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if add is near Akama
        float distanceToAkama = unit->GetDistance(akama);
        if (distanceToAkama < 15.0f)
        {
            float distanceToBot = bot->GetDistance(unit);
            if (distanceToBot < nearestDistance)
            {
                nearestThreat = unit;
                nearestDistance = distanceToBot;
            }
        }
    }

    if (nearestThreat)
    {
        // Switch to threat near Akama
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget != nearestThreat)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestThreat);
            bot->SetTarget(nearestThreat->GetGUID());
        }
        return true;
    }

    return false;
}

bool ShadeOfAkamaPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Position based on phase
    if (g_shade_phase[botGuid] == 1)
    {
        // Phase 1: Spread out near channelers
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        bool tooClose = false;
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;

            float distance = bot->GetDistance(member);
            if (distance < 5.0f)
            {
                tooClose = true;
                break;
            }
        }

        if (tooClose)
        {
            // Spread out
            float angle = bot->GetOrientation() + (float(rand()) / RAND_MAX) * M_PI * 2;
            float moveDistance = 8.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }
    else if (g_shade_phase[botGuid] == 2)
    {
        // Phase 2: Position around shade
        Unit* shade = AI_VALUE2(Unit*, "find target", "shade of akama");
        if (shade && shade->IsAlive())
        {
            float distance = bot->GetDistance(shade);
            float optimalDistance = botAI->IsRanged(bot) ? 20.0f : 5.0f;
            
            if (fabs(distance - optimalDistance) > 3.0f)
            {
                float angle = bot->GetAngle(shade);
                float moveDistance = distance - optimalDistance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

// Teron Gorefiend
static std::map<ObjectGuid, bool> g_teron_hasGhost;
static std::map<ObjectGuid, uint32> g_teron_lastBlossomMove;

bool TeronGorefiendShadowOfDeathAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if we have Shadow of Death debuff
    const uint32 SPELL_SHADOW_OF_DEATH = 40251;
    if (!bot->HasAura(SPELL_SHADOW_OF_DEATH))
    {
        g_teron_hasGhost[botGuid] = false;
        return false;
    }

    // Healers should prioritize healing Shadow of Death targets
    if (botAI->IsHeal(bot))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;

            // Check if member has Shadow of Death
            if (member->HasAura(SPELL_SHADOW_OF_DEATH))
            {
                float healthPercent = member->GetHealthPct();
                if (healthPercent < 50.0f)
                {
                    // Priority heal target with Shadow of Death
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(member);
                    return true;
                }
            }
        }
    }

    // Move away from others to avoid spreading damage when we die
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 10.0f)
        {
            // Move away from allies
            float angle = bot->GetAngle(member) + M_PI;
            float moveDistance = 15.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool TeronGorefiendGhostFormAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if we're in ghost form (have Spiritual Vengeance)
    const uint32 SPELL_SPIRITUAL_VENGEANCE = 40268;
    const uint32 SPELL_SPIRIT_LANCE = 40157;
    const uint32 SPELL_SPIRIT_CHAINS = 40175;
    const uint32 SPELL_SPIRIT_VOLLEY = 40314;
    
    if (!bot->HasAura(SPELL_SPIRITUAL_VENGEANCE))
    {
        g_teron_hasGhost[botGuid] = false;
        return false;
    }

    g_teron_hasGhost[botGuid] = true;

    // In ghost form, we need to use special abilities on the shadowy constructs
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* nearestConstruct = nullptr;
    float nearestDistance = 50.0f;
    
    const uint32 NPC_SHADOWY_CONSTRUCT = 23111;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SHADOWY_CONSTRUCT)
        {
            float distance = bot->GetDistance(unit);
            if (distance < nearestDistance)
            {
                nearestConstruct = unit;
                nearestDistance = distance;
            }
        }
    }

    if (nearestConstruct)
    {
        // Use ghost abilities on constructs
        if (botAI->IsRanged(bot))
        {
            if (nearestDistance > 5.0f && nearestDistance < 30.0f)
            {
                bot->CastSpell(nearestConstruct, SPELL_SPIRIT_LANCE, false);
            }
            else if (nearestDistance < 20.0f)
            {
                bot->CastSpell(nearestConstruct, SPELL_SPIRIT_VOLLEY, false);
            }
        }
        else
        {
            if (nearestDistance < 10.0f)
            {
                bot->CastSpell(nearestConstruct, SPELL_SPIRIT_CHAINS, false);
            }
        }
        
        // Make sure we're targeting the construct
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget != nearestConstruct)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestConstruct);
            bot->SetTarget(nearestConstruct->GetGUID());
        }
        return true;
    }

    return false;
}

bool TeronGorefiendDoomBlossomAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    uint32 currentTime = time(nullptr);
    
    // Don't move too frequently
    if (g_teron_lastBlossomMove[botGuid] > 0 && 
        (currentTime - g_teron_lastBlossomMove[botGuid]) < 2)
        return false;

    // Check for Doom Blossom creatures
    const uint32 NPC_DOOM_BLOSSOM = 23123;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_DOOM_BLOSSOM)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 20.0f)  // Doom Blossom has 20 yard shadow damage aura
            {
                // Move away from Doom Blossom
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = 25.0f - distance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                g_teron_lastBlossomMove[botGuid] = currentTime;
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool TeronGorefiendIncinerateSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Check if we have Incinerate debuff
    const uint32 SPELL_INCINERATE = 40239;
    if (!bot->HasAura(SPELL_INCINERATE))
        return false;

    // Spread out from other players to avoid splash damage
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f)
        {
            // Move away from allies
            float angle = bot->GetAngle(member) + M_PI;
            float moveDistance = 10.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool TeronGorefiendCrushingShadowsSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Crushing Shadows
    const uint32 SPELL_CRUSHING_SHADOWS = 40243;
    if (!boss->FindCurrentSpellBySpellId(SPELL_CRUSHING_SHADOWS))
        return false;

    // Spread out to minimize targets hit
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    int nearbyCount = 0;
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 10.0f)
            nearbyCount++;
    }

    // If too many people nearby, spread out
    if (nearbyCount > 1)
    {
        float angle = bot->GetOrientation() + (float(rand()) / RAND_MAX) * M_PI * 2;
        float moveDistance = 12.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

// Gurtogg Bloodboil
static std::map<ObjectGuid, uint32> g_gurtogg_lastPositionTime;
static std::map<ObjectGuid, bool> g_gurtogg_hasFelRage;
static std::map<ObjectGuid, uint32> g_gurtogg_lastGeyserMove;

bool GurtoggBloodboilPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    uint32 currentTime = time(nullptr);
    
    // Don't reposition too frequently
    if (g_gurtogg_lastPositionTime[botGuid] > 0 && 
        (currentTime - g_gurtogg_lastPositionTime[botGuid]) < 3)
        return false;

    // During normal phase, position for Bloodboil mechanic
    const uint32 SPELL_FEL_RAGE_SELF = 40594;
    if (!boss->HasAura(SPELL_FEL_RAGE_SELF))
    {
        float distance = bot->GetDistance(boss);
        
        // Bloodboil hits the 5 furthest targets
        // Ranged should stay at max range to absorb it
        // Melee should stay close unless they need to soak
        if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
        {
            // Ranged/healers should be at 30-35 yards to soak Bloodboil
            if (distance < 30.0f || distance > 35.0f)
            {
                float targetDistance = 32.0f;
                float angle = bot->GetAngle(boss);
                float moveDistance = targetDistance - distance;
                
                float newX = bot->GetPositionX() - cos(angle) * moveDistance;
                float newY = bot->GetPositionY() - sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                g_gurtogg_lastPositionTime[botGuid] = currentTime;
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else if (botAI->IsTank(bot))
        {
            // Tanks should be in melee range
            if (distance > 5.0f)
            {
                float angle = bot->GetAngle(boss);
                float moveDistance = distance - 3.0f;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                g_gurtogg_lastPositionTime[botGuid] = currentTime;
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else
        {
            // Melee DPS should be behind boss
            float optimalDistance = 5.0f;
            if (fabs(distance - optimalDistance) > 2.0f)
            {
                float angle = boss->GetOrientation() + M_PI;
                float newX = boss->GetPositionX() + cos(angle) * optimalDistance;
                float newY = boss->GetPositionY() + sin(angle) * optimalDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                g_gurtogg_lastPositionTime[botGuid] = currentTime;
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool GurtoggFelRageTargetAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // Check if we have Fel Rage
    const uint32 SPELL_FEL_RAGE_TARGET = 40604;
    const uint32 SPELL_FEL_RAGE_2 = 40616;
    const uint32 SPELL_FEL_RAGE_3 = 41625;
    
    if (!bot->HasAura(SPELL_FEL_RAGE_TARGET) && 
        !bot->HasAura(SPELL_FEL_RAGE_2) && 
        !bot->HasAura(SPELL_FEL_RAGE_3))
    {
        g_gurtogg_hasFelRage[botGuid] = false;
        return false;
    }

    g_gurtogg_hasFelRage[botGuid] = true;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // During Fel Rage, we must tank the boss
    // Use taunt if boss is not targeting us
    if (boss->GetVictim() != bot)
    {
        // Try to taunt
        const uint32 SPELL_TAUNT_GURTOGG = 40603;
        bot->CastSpell(boss, SPELL_TAUNT_GURTOGG, false);
    }

    // Make sure we're targeting the boss
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget != boss)
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(boss);
        bot->SetTarget(boss->GetGUID());
    }

    // Position in front of boss
    float distance = bot->GetDistance(boss);
    if (distance > 5.0f)
    {
        float angle = bot->GetAngle(boss);
        float moveDistance = distance - 3.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
    }

    return true;
}

bool GurtoggArcingSmashAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Arcing Smash
    const uint32 SPELL_ARCING_SMASH1 = 40457;
    const uint32 SPELL_ARCING_SMASH2 = 40599;
    
    if (!boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH1) &&
        !boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH2))
        return false;

    // Non-tanks should avoid frontal cone
    if (!botAI->IsTank(bot))
    {
        float angle = boss->GetOrientation();
        float botAngle = boss->GetAngle(bot);
        float angleDiff = fabs(angle - botAngle);
        
        // If we're in front (within 90 degree cone), move to side
        if (angleDiff < M_PI / 2)
        {
            // Move to the side
            float moveAngle = angle + (botAngle > angle ? M_PI / 2 : -M_PI / 2);
            float moveDistance = 10.0f;
            
            float newX = boss->GetPositionX() + cos(moveAngle) * moveDistance;
            float newY = boss->GetPositionY() + sin(moveAngle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool GurtoggFelAcidBreathAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Fel Acid Breath
    const uint32 SPELL_FEL_ACID_BREATH1 = 40508;
    const uint32 SPELL_FEL_ACID_BREATH2 = 40595;
    
    if (!boss->FindCurrentSpellBySpellId(SPELL_FEL_ACID_BREATH1) &&
        !boss->FindCurrentSpellBySpellId(SPELL_FEL_ACID_BREATH2))
        return false;

    // Non-tanks should avoid frontal cone
    if (!botAI->IsTank(bot))
    {
        float angle = boss->GetOrientation();
        float botAngle = boss->GetAngle(bot);
        float angleDiff = fabs(angle - botAngle);
        
        // If we're in front (within 90 degree cone), move behind
        if (angleDiff < M_PI / 2)
        {
            // Move behind boss
            float moveAngle = angle + M_PI;
            float moveDistance = 8.0f;
            
            float newX = boss->GetPositionX() + cos(moveAngle) * moveDistance;
            float newY = boss->GetPositionY() + sin(moveAngle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool GurtoggFelGeyserAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    uint32 currentTime = time(nullptr);
    
    // Don't move too frequently
    if (g_gurtogg_lastGeyserMove[botGuid] > 0 && 
        (currentTime - g_gurtogg_lastGeyserMove[botGuid]) < 2)
        return false;

    // Check for Fel Geyser creatures
    const uint32 NPC_FEL_GEYSER = 23254;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FEL_GEYSER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)  // Fel Geyser damage radius
            {
                // Move away from geyser
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = 15.0f - distance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                g_gurtogg_lastGeyserMove[botGuid] = currentTime;
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool GurtoggAcidicWoundTankSwapAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Only for tanks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // Check current tank's Acidic Wound stacks
    const uint32 SPELL_ACIDIC_WOUND = 40484;
    Unit* currentTank = boss->GetVictim();
    
    if (currentTank && currentTank != bot)
    {
        // Check if current tank has too many stacks
        if (Aura* aura = currentTank->GetAura(SPELL_ACIDIC_WOUND))
        {
            if (aura->GetStackAmount() >= 3)
            {
                // Taunt to take over tanking
                uint32 tauntSpellId = 355; // Taunt (Warrior)
                if (bot->getClass() == CLASS_PALADIN)
                    tauntSpellId = 62124; // Hand of Reckoning
                else if (bot->getClass() == CLASS_DRUID)
                    tauntSpellId = 6795; // Growl
                else if (bot->getClass() == CLASS_DEATH_KNIGHT)
                    tauntSpellId = 56222; // Dark Command
                
                bot->CastSpell(boss, tauntSpellId, false);
                return true;
            }
        }
    }
    else if (currentTank == bot)
    {
        // Check our own stacks
        if (Aura* aura = bot->GetAura(SPELL_ACIDIC_WOUND))
        {
            if (aura->GetStackAmount() >= 5)
            {
                // We have too many stacks, another tank should taunt
                // Call for help in raid
                return false;
            }
        }
    }

    return false;
}

// Reliquary of Souls
static std::map<ObjectGuid, uint32> g_reliquary_phase;
static std::map<ObjectGuid, uint32> g_reliquary_lastPositionTime;
static std::map<ObjectGuid, uint32> g_reliquary_lastSoulTarget;

bool ReliquaryEssenceOfSufferingPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // During Essence of Suffering, position based on fixate mechanic
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering || !suffering->IsAlive())
        return false;

    uint32 currentTime = time(nullptr);
    
    // Don't reposition too frequently
    if (g_reliquary_lastPositionTime[botGuid] > 0 && 
        (currentTime - g_reliquary_lastPositionTime[botGuid]) < 3)
        return false;

    float distance = bot->GetDistance(suffering);
    
    // Suffering fixates on nearest player
    // Tanks should be closest, others should stay back
    if (botAI->IsTank(bot))
    {
        // Tanks should be in melee range to get fixated
        if (distance > 5.0f)
        {
            float angle = bot->GetAngle(suffering);
            float moveDistance = distance - 3.0f;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            g_reliquary_lastPositionTime[botGuid] = currentTime;
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }
    else
    {
        // Non-tanks should stay at range to avoid fixate
        if (distance < 20.0f)
        {
            float angle = bot->GetAngle(suffering) + M_PI;
            float moveDistance = 25.0f - distance;
            
            float newX = bot->GetPositionX() + cos(angle) * moveDistance;
            float newY = bot->GetPositionY() + sin(angle) * moveDistance;
            float newZ = bot->GetPositionZ();
            
            bot->UpdateGroundPositionZ(newX, newY, newZ);
            g_reliquary_lastPositionTime[botGuid] = currentTime;
            
            return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }

    g_reliquary_phase[botGuid] = 1;
    return false;
}

bool ReliquaryEssenceOfDesireAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire || !desire->IsAlive())
        return false;

    // During Essence of Desire, be careful with healing
    // Aura of Desire reflects healing as damage
    const uint32 SPELL_AURA_OF_DESIRE = 41350;
    const uint32 SPELL_RUNE_SHIELD = 41431;
    
    // Check if Desire has Rune Shield (spell immunity)
    if (desire->HasAura(SPELL_RUNE_SHIELD))
    {
        // Can't damage during Rune Shield, focus on positioning
        float distance = bot->GetDistance(desire);
        if (botAI->IsRanged(bot))
        {
            if (distance < 20.0f)
            {
                float angle = bot->GetAngle(desire) + M_PI;
                float moveDistance = 25.0f - distance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    // Healers need to be very careful
    if (botAI->IsHeal(bot))
    {
        // Only heal critical targets during Desire phase
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
            return false;
            
        GuidVector members = membersValue->Get();
        
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;

            // Only heal if below 30% health
            if (member->GetHealthPct() < 30.0f)
            {
                // Be aware healing will reflect damage back
                return false;
            }
        }
    }

    g_reliquary_phase[botGuid] = 2;
    return false;
}

bool ReliquaryEssenceOfAngerSpiteAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Check for Spite targets and avoid them
    const uint32 SPELL_SPITE = 41376;
    
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        // Check if member has Spite (will explode on removal)
        if (member->HasAura(SPELL_SPITE))
        {
            float distance = bot->GetDistance(member);
            if (distance < 15.0f)
            {
                // Move away from Spite target
                float angle = bot->GetAngle(member) + M_PI;
                float moveDistance = 20.0f - distance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool ReliquaryPhaseTransitionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    // During phase transitions, focus on adds
    const uint32 NPC_ENSLAVED_SOUL = 23469;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* nearestSoul = nullptr;
    float nearestDistance = 50.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ENSLAVED_SOUL)
        {
            float distance = bot->GetDistance(unit);
            if (distance < nearestDistance)
            {
                nearestSoul = unit;
                nearestDistance = distance;
            }
        }
    }

    if (nearestSoul)
    {
        // Position based on role
        if (botAI->IsTank(bot))
        {
            // Tanks should pick up adds
            if (nearestDistance > 5.0f)
            {
                float angle = bot->GetAngle(nearestSoul);
                float moveDistance = nearestDistance - 3.0f;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else if (botAI->IsRanged(bot))
        {
            // Ranged should maintain distance
            if (nearestDistance < 15.0f)
            {
                float angle = bot->GetAngle(nearestSoul) + M_PI;
                float moveDistance = 20.0f - nearestDistance;
                
                float newX = bot->GetPositionX() + cos(angle) * moveDistance;
                float newY = bot->GetPositionY() + sin(angle) * moveDistance;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool ReliquaryEnslavedSoulAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
        return false;

    const uint32 NPC_ENSLAVED_SOUL = 23469;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    Unit* nearestSoul = nullptr;
    float nearestDistance = 50.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ENSLAVED_SOUL)
        {
            float distance = bot->GetDistance(unit);
            if (distance < nearestDistance)
            {
                nearestSoul = unit;
                nearestDistance = distance;
            }
        }
    }

    if (nearestSoul)
    {
        // Switch to attacking the add
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget != nearestSoul)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestSoul);
            bot->SetTarget(nearestSoul->GetGUID());
            g_reliquary_lastSoulTarget[botGuid] = time(nullptr);
        }
        return true;
    }

    return false;
}

bool ReliquarySoulScreamSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (!anger || !anger->IsAlive())
        return false;

    // Check if casting Soul Scream
    const uint32 SPELL_SOUL_SCREAM = 41545;
    if (!anger->FindCurrentSpellBySpellId(SPELL_SOUL_SCREAM))
        return false;

    // Spread out to minimize Soul Scream damage
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    int nearbyCount = 0;
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f)
            nearbyCount++;
    }

    // If too many people nearby, spread out
    if (nearbyCount > 1)
    {
        float angle = bot->GetOrientation() + (float(rand()) / RAND_MAX) * M_PI * 2;
        float moveDistance = 10.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

// Mother Shahraz
static std::map<ObjectGuid, bool> g_shahraz_hasFatalAttraction;
static std::map<ObjectGuid, time_t> g_shahraz_lastAttractionTime;

bool MotherShahrazSaberLashPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Only for tanks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!boss || !boss->IsAlive())
        return false;

    // Find other tanks and stack with them for Saber Lash
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        Player* memberPlayer = member->ToPlayer();
        if (memberPlayer && botAI->IsTank(memberPlayer))
        {
            float distance = bot->GetDistance(member);
            if (distance > 5.0f)
            {
                // Move to stack with other tank
                return MoveTo(bot->GetMapId(), member->GetPositionX(), member->GetPositionY(), 
                            member->GetPositionZ(), false, false, false, true,
                            MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool MotherShahrazFatalAttractionAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    
    // Check if we have Fatal Attraction
    const uint32 SPELL_FATAL_ATTRACTION_AURA = 41001;
    if (!bot->HasAura(SPELL_FATAL_ATTRACTION_AURA))
    {
        g_shahraz_hasFatalAttraction[botGuid] = false;
        return false;
    }

    g_shahraz_hasFatalAttraction[botGuid] = true;
    g_shahraz_lastAttractionTime[botGuid] = time(nullptr);

    // Spread out from other Fatal Attraction targets
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        if (member->HasAura(SPELL_FATAL_ATTRACTION_AURA))
        {
            float distance = bot->GetDistance(member);
            if (distance < 15.0f)
            {
                // Emergency spread
                float angle = bot->GetAngle(member) + M_PI;
                float newX = bot->GetPositionX() + cos(angle) * 20.0f;
                float newY = bot->GetPositionY() + sin(angle) * 20.0f;
                float newZ = bot->GetPositionZ();
                
                bot->UpdateGroundPositionZ(newX, newY, newZ);
                
                return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool MotherShahrazBeamAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    // Check for beam debuffs
    const uint32 beamSpells[] = {40860, 40861, 40859, 40827}; // Sinful, Sinister, Vile, Wicked
    
    bool hasBeam = false;
    for (uint32 spellId : beamSpells)
    {
        if (bot->HasAura(spellId))
        {
            hasBeam = true;
            break;
        }
    }

    if (!hasBeam)
        return false;

    // Spread from other players when beamed
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    int nearbyCount = 0;
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f)
            nearbyCount++;
    }

    // If too many people nearby, spread out
    if (nearbyCount > 1)
    {
        float angle = bot->GetOrientation() + (float(rand()) / RAND_MAX) * M_PI * 2;
        float moveDistance = 10.0f;
        
        float newX = bot->GetPositionX() + cos(angle) * moveDistance;
        float newY = bot->GetPositionY() + sin(angle) * moveDistance;
        float newZ = bot->GetPositionZ();
        
        bot->UpdateGroundPositionZ(newX, newY, newZ);
        
        return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool MotherShahrazPrismaticResistanceAction::Execute(Event event)
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!boss || !boss->IsAlive())
        return false;

    // Track which prismatic aura the boss has
    const uint32 prismaticAuras[] = {
        40880, // Shadow
        40882, // Fire
        40883, // Nature
        40891, // Arcane
        40896, // Frost
        40897  // Holy
    };

    for (uint32 auraId : prismaticAuras)
    {
        if (boss->HasAura(auraId))
        {
            // Could swap gear or use resistance consumables here
            // For now just track it
            return true;
        }
    }

    return false;
}