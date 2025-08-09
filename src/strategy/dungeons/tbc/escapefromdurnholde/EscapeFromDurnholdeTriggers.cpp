#include "EscapeFromDurnholdeTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "AttackersValue.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_F
#define M_PI_F 3.14159265f
#endif

bool ThrallLowHealthTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    Creature* thrall = bot->FindNearestCreature(NPC_THRALL, 100.0f);
    if (!thrall || !thrall->IsAlive())
        return false;
    
    return thrall->GetHealthPct() < 60.0f;
}

bool ThrallCriticalHealthTrigger::IsActive() 
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    Creature* thrall = bot->FindNearestCreature(NPC_THRALL, 100.0f);
    if (!thrall || !thrall->IsAlive())
        return false;
    
    return thrall->GetHealthPct() < 25.0f;
}

bool LieutenantDrakeWhirlwindTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    Unit* drake = AI_VALUE2(Unit*, "find target", "lieutenant drake");
    if (!drake || !drake->IsAlive())
        return false;
    
    // Check if Drake is CASTING whirlwind (not just has aura)
    // Whirlwind is a channeled spell that lasts 4 seconds
    if (drake->HasUnitState(UNIT_STATE_CASTING) && drake->FindCurrentSpellBySpellId(SPELL_DRAKE_WHIRLWIND))
        return true;
        
    // Also check for the aura in case it leaves one
    if (drake->HasAura(SPELL_DRAKE_WHIRLWIND))
        return true;
    
    // Check if bot is too close during any melee swing that could be whirlwind
    // This is a fallback detection in case the above methods fail
    if (drake->HasUnitState(UNIT_STATE_MELEE_ATTACKING) && bot->GetDistance(drake) < 8.0f)
    {
        // Check if drake recently cast whirlwind (within 6 seconds)
        // This helps maintain avoidance even if we can't detect the spell directly
        static std::map<ObjectGuid, uint32> lastWhirlwindTime;
        uint32 currentTime = getMSTime();
        
        if (drake->HasUnitState(UNIT_STATE_CASTING))
        {
            lastWhirlwindTime[drake->GetGUID()] = currentTime;
            return true;
        }
        
        // If we saw a cast recently, stay away for 6 seconds
        if (lastWhirlwindTime[drake->GetGUID()] > 0 && 
            (currentTime - lastWhirlwindTime[drake->GetGUID()]) < 6000)
        {
            return true;
        }
    }
    
    return false;
}

bool CaptainSkarlocHammerOfJusticeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    return bot->HasAura(SPELL_SKARLOC_HAMMER);
}

bool EpochHunterSandBreathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    
    Unit* epoch = AI_VALUE2(Unit*, "find target", "epoch hunter");
    if (!epoch || !epoch->IsAlive())
        return false;
    
    if (bot->GetDistance(epoch) > 15.0f)
        return false;
    
    // Check if bot is in front of Epoch Hunter (Sand Breath cone)
    return epoch->HasInArc(M_PI / 3.0f, bot);
}

bool EpochHunterMagicDisruptionAuraTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;
    return bot->HasAura(SPELL_EPOCH_MAGIC_DISRUPTION);
}

bool EfdReturnPositionTrigger::IsActive()
{
    // Delegate to the action's isUseful method
    // This trigger is active when we need to return from a safe position
    return false; // Will be handled by action's isUseful
}