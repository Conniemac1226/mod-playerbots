#include "ManaTombsMultipliers.h"
#include "ManaTombsActions.h"
#include "Ai/Base/Actions/GenericActions.h"
#include "Ai/Base/Actions/GenericSpellActions.h"
#include "ChooseTargetActions.h"

float PandemoniusDarkShellMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pandemonius");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }

    bool darkShellActive = boss->HasAura(SPELL_DARK_SHELL) ||
        (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_DARK_SHELL));
    if (!darkShellActive)
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) || dynamic_cast<PetAttackAction*>(action))
    {
        return 0.0f;
    }

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction && !dynamic_cast<CastHealingSpellAction*>(action))
    {
        if (castSpellAction->GetTarget() == boss ||
            castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float EtherealBeaconMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Ethereal Beacons are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Ethereal Beacons using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool beaconPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_ETHEREAL_BEACON)
        {
            beaconPresent = true;
            break;
        }
    }
    
    if (beaconPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when beacons present
    }
    
    return 1.0f;
}
