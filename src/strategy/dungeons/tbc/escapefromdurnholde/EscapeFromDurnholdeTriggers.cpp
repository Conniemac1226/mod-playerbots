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
    
    return drake->HasAura(SPELL_DRAKE_WHIRLWIND);
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