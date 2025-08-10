#include "MagtheridonStrategy.h"
#include "MagtheridonMultipliers.h"

void MagtheridonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Core mechanics that apply to all phases
    triggers.push_back(new TriggerNode(
        "blast nova cast",
        NextAction::array(0, new NextAction("click manticron cube", ACTION_EMERGENCY), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "stop cube channel",
        NextAction::array(0, new NextAction("stop clicking cube", ACTION_EMERGENCY - 1.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "quake cast",
        NextAction::array(0, new NextAction("avoid quake", ACTION_EMERGENCY - 5.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "debris falling",
        NextAction::array(0, new NextAction("avoid debris", ACTION_EMERGENCY - 10.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "blaze nearby",
        NextAction::array(0, new NextAction("avoid blaze", ACTION_INTERRUPT + 5.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "infernal nearby",
        NextAction::array(0, new NextAction("avoid infernal", ACTION_INTERRUPT + 5.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "cleave danger",
        NextAction::array(0, new NextAction("spread for cleave", ACTION_MOVE), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "healer position needed",
        NextAction::array(0, new NextAction("healer position magtheridon", ACTION_MOVE), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "banish phase active",
        NextAction::array(0, new NextAction("banish phase position", ACTION_MOVE), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "magtheridon phase transition",
        NextAction::array(0, new NextAction("magtheridon phase transition", ACTION_NORMAL), nullptr)));
}

void MagtheridonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BlastNovaMultiplier(botAI));
    multipliers.push_back(new ChannelerInterruptMultiplier(botAI));
    multipliers.push_back(new MagtheridonMovementMultiplier(botAI));
    multipliers.push_back(new MagtheridonAddsMultiplier(botAI));
    multipliers.push_back(new MagtheridonTankMultiplier(botAI));
    multipliers.push_back(new MagtheridonHealerMultiplier(botAI));
}

void MagtheridonChannelersStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 1: Kill channelers
    triggers.push_back(new TriggerNode(
        "hellfire channeler near",
        NextAction::array(0, new NextAction("hellfire channeler target", ACTION_INTERRUPT + 10.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "hellfire warder active",
        NextAction::array(0, new NextAction("hellfire warder target", ACTION_INTERRUPT + 5.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "dark mending cast",
        NextAction::array(0, new NextAction("interrupt dark mending", ACTION_INTERRUPT), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "shadow bolt volley cast",
        NextAction::array(0, new NextAction("coordinate channeler interrupt", ACTION_INTERRUPT), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "channeler interrupt needed",
        NextAction::array(0, new NextAction("coordinate channeler interrupt", ACTION_INTERRUPT), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "tank position adds",
        NextAction::array(0, new NextAction("tank position adds", ACTION_INTERRUPT), nullptr)));
}

void MagtheridonReleasedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 2: Magtheridon released
    triggers.push_back(new TriggerNode(
        "magtheridon released",
        NextAction::array(0, new NextAction("attack magtheridon", ACTION_NORMAL + 10.0f), nullptr)));
        
    // Blast Nova becomes critical
    triggers.push_back(new TriggerNode(
        "blast nova cast",
        NextAction::array(0, new NextAction("click manticron cube", ACTION_EMERGENCY), nullptr)));
        
    // Mind exhaustion management
    triggers.push_back(new TriggerNode(
        "mind exhaustion check",
        NextAction::array(0, new NextAction("wait for exhaustion", ACTION_NORMAL), nullptr)));
}

void MagtheridonCaveInStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 3: Cave in at 30%
    triggers.push_back(new TriggerNode(
        "cave in",
        NextAction::array(0, new NextAction("handle cave in", ACTION_EMERGENCY + 2.0f), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "magtheridon low health",
        NextAction::array(0, new NextAction("burn phase", ACTION_NORMAL + 5.0f), nullptr)));
        
    // Debris becomes more frequent
    triggers.push_back(new TriggerNode(
        "debris falling",
        NextAction::array(0, new NextAction("avoid debris", ACTION_EMERGENCY), nullptr)));
}