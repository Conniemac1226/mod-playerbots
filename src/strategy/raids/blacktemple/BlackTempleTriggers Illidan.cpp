#include "BlackTempleTriggers.h"
#include "AiObjectContext.h"
#include "GameObject.h"
#include "Unit.h"
#include "Value.h"
#include "Item.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Illidan Stormrage
bool IllidanEngagedTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->IsAlive() && illidan->IsInCombat();
}

bool IllidanFlameCrashTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if Illidan is casting Flame Crash
    const uint32 SPELL_FLAME_CRASH = 40832;
    return illidan->FindCurrentSpellBySpellId(SPELL_FLAME_CRASH);
}

bool IllidanParasiticShadowfiendTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for Parasitic Shadowfiends
    const uint32 NPC_PARASITIC_SHADOWFIEND = 23498;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PARASITIC_SHADOWFIEND)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 40.0f)
                return true;
        }
    }

    return false;
}

bool IllidanDrawSoulTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we have Draw Soul debuff
    const uint32 SPELL_DRAW_SOUL = 40904;
    return bot->HasAura(SPELL_DRAW_SOUL);
}

bool IllidanAgonizingFlamesTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for Agonizing Flames ground effect
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        float distance = bot->GetDistance(go);
        if (distance < 8.0f)
            return true;
    }

    return false;
}

bool IllidanEyeBeamTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if in Phase 2 (flying) and casting Eye Beam
    if (!illidan->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
        return false;

    const uint32 SPELL_EYE_BLAST = 39908;
    return illidan->FindCurrentSpellBySpellId(SPELL_EYE_BLAST);
}

bool IllidanDarkBarrageTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if casting Dark Barrage
    const uint32 SPELL_DARK_BARRAGE = 40585;
    return illidan->FindCurrentSpellBySpellId(SPELL_DARK_BARRAGE);
}

bool IllidanFlameOfAzzinothTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for Flame of Azzinoth adds
    const uint32 NPC_FLAME_OF_AZZINOTH = 22997;
    
    if (!botAI->IsTank(bot))
        return false;

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FLAME_OF_AZZINOTH)
        {
            // Check if it needs a tank
            Unit* currentTarget = unit->GetVictim();
            if (!currentTarget)
                return true;
            
            // Check if current target is a tank (must be a player)
            Player* targetPlayer = currentTarget->ToPlayer();
            if (!targetPlayer || !botAI->IsTank(targetPlayer))
                return true;
        }
    }

    return false;
}

bool IllidanShadowDemonTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for Shadow Demons
    const uint32 NPC_SHADOW_DEMON = 23375;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SHADOW_DEMON)
        {
            // Priority if it's targeting us
            if (unit->GetVictim() == bot)
                return true;
                
            float distance = bot->GetDistance(unit);
            if (distance < 50.0f)
                return true;
        }
    }

    return false;
}

bool IllidanDemonFormTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if Illidan is in Demon Form
    const uint32 SPELL_DEMON_FORM = 40506;
    return illidan->HasAura(SPELL_DEMON_FORM);
}

bool IllidanCageTrapTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for cage traps
    const uint32 GO_CAGE_TRAP = 185916;
    
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || go->GetEntry() != GO_CAGE_TRAP)
            continue;

        float distance = bot->GetDistance(go);
        if (distance < 10.0f)
            return true;
    }

    return false;
}

bool IllidanEnrageTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || !illidan->IsAlive())
        return false;

    // Check if in Phase 5 (Frenzy/Enrage)
    const uint32 SPELL_FRENZY = 40683;
    if (!illidan->HasAura(SPELL_FRENZY))
        return false;

    // Only for tanks
    return botAI->IsTank(bot) && illidan->GetVictim() == bot;
}