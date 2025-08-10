#include "MagtheridonMultipliers.h"
#include "MagtheridonActions.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"

float BlastNovaMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;
        
    // Blast Nova is highest priority
    if (magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        if (dynamic_cast<ClickManticronCubeAction*>(action))
            return 0.95f; // Priority 90-94 range for emergency
            
        if (dynamic_cast<StopClickingCubeAction*>(action))
            return 0.9f;
    }
    
    return 1.0f;
}

float ChannelerInterruptMultiplier::GetValue(Action* action)
{
    // Interrupts are high priority
    if (dynamic_cast<InterruptDarkMendingAction*>(action))
        return 0.75f; // Priority 70-74 range for interrupts
        
    if (dynamic_cast<CoordinateChannelerInterruptAction*>(action))
        return 0.74f;
    
    return 1.0f;
}

float MagtheridonMovementMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;
        
    // Cave in is emergency priority
    if (magtheridon->GetHealthPct() <= 30.0f)
    {
        if (dynamic_cast<HandleCaveInAction*>(action))
            return 0.92f; // Emergency movement
    }
    
    // Quake avoidance is high priority
    if (magtheridon->FindCurrentSpellBySpellId(SPELL_QUAKE))
    {
        if (dynamic_cast<AvoidQuakeAction*>(action))
            return 0.85f; // High priority movement
    }
    
    // Debris avoidance
    if (dynamic_cast<AvoidDebrisAction*>(action))
        return 0.8f;
        
    // Blaze avoidance
    if (dynamic_cast<AvoidBlazeAction*>(action))
        return 0.75f;
        
    // Infernal avoidance
    if (dynamic_cast<AvoidInfernalAction*>(action))
        return 0.75f;
        
    // General positioning
    if (dynamic_cast<SpreadForCleaveAction*>(action))
        return 0.35f; // Normal movement priority
        
    if (dynamic_cast<HealerPositionMagtheridonAction*>(action))
        return 0.35f;
        
    if (dynamic_cast<BanishPhasePositionAction*>(action))
        return 0.35f;
    
    return 1.0f;
}

float MagtheridonAddsMultiplier::GetValue(Action* action)
{
    // Channelers and Warders are priority targets
    if (dynamic_cast<HellfireChannelerTargetAction*>(action))
        return 0.8f; // High priority targeting
        
    if (dynamic_cast<HellfireWarderTargetAction*>(action))
        return 0.75f;
        
    if (dynamic_cast<TankPositionAddsAction*>(action))
        return 0.7f;
    
    return 1.0f;
}

float MagtheridonTankMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;
        
    // Tank-specific priorities
    if (dynamic_cast<TankPositionAddsAction*>(action))
        return 0.8f; // Important for tanks
        
    // Tanks should not click cubes
    if (dynamic_cast<ClickManticronCubeAction*>(action))
        return 1.5f; // Lower priority for tanks
    
    return 1.0f;
}

float MagtheridonHealerMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsHeal(bot))
        return 1.0f;
        
    // Healer positioning is important
    if (dynamic_cast<HealerPositionMagtheridonAction*>(action))
        return 0.7f; // Higher priority for healers
        
    // Healers can help with cubes if assigned
    if (dynamic_cast<ClickManticronCubeAction*>(action))
    {
        uint32 subGroup = bot->GetSubGroup();
        if (subGroup >= 3 && subGroup < 5) // Groups 3-4 for healers
            return 0.9f;
    }
    
    return 1.0f;
}