#include "MechanarStrategy.h"
#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "MechanarMultipliers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

MechanarStrategy::MechanarStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}

void MechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ========== MECHANO LORD CAPACITUS ==========
    
    // Reflective shields (Normal mode)
    triggers.push_back(new TriggerNode(
        "reflective shield active",
        NextAction::array(0, new NextAction("handle reflective shield", ACTION_INTERRUPT + 1), nullptr)));
    
    // Polarity Shift (Heroic mode)
    triggers.push_back(new TriggerNode(
        "polarity shift active",
        NextAction::array(0, new NextAction("handle polarity shift", ACTION_EMERGENCY), nullptr)));
    
    // Nether Charges
    triggers.push_back(new TriggerNode(
        "nether charge active",
        NextAction::array(0, new NextAction("attack nether charge", ACTION_NORMAL + 5), nullptr)));
    
    // General positioning for Capacitus
    triggers.push_back(new TriggerNode(
        "capacitus engaged",
        NextAction::array(0, new NextAction("capacitus position", ACTION_MOVE + 1), nullptr)));
    
    // ========== NETHERMANCER SEPETHREA ==========
    
    // Universal fire trail avoidance
    triggers.push_back(new TriggerNode(
        "raging flames fire trail",
        NextAction::array(0, new NextAction("avoid raging flames fire trail", ACTION_EMERGENCY + 4), nullptr)));
    
    // Raging Flames fixate
    triggers.push_back(new TriggerNode(
        "raging flames active",
        NextAction::array(0, new NextAction("flee raging flames", ACTION_EMERGENCY + 1), nullptr)));
        
    // Inferno AoE avoidance
    triggers.push_back(new TriggerNode(
        "raging flames inferno",
        NextAction::array(0, new NextAction("avoid raging flames inferno", ACTION_EMERGENCY + 4), nullptr)));
    
    // Raging Flames targeting
    triggers.push_back(new TriggerNode(
        "should target raging flames",
        NextAction::array(0, new NextAction("target raging flames", ACTION_NORMAL + 8), nullptr)));
    
    // Dragon's Breath frontal cone
    triggers.push_back(new TriggerNode(
        "dragons breath danger",
        NextAction::array(0, new NextAction("avoid dragons breath", ACTION_MOVE + 4), nullptr)));
    
    
    // Arcane Blast threat reduction
    triggers.push_back(new TriggerNode(
        "sepethrea engaged",
        NextAction::array(0, new NextAction("handle arcane blast", ACTION_NORMAL + 2), nullptr)));
    
    // ========== PATHALEON THE CALCULATOR ==========
    
    // Mind Control
    triggers.push_back(new TriggerNode(
        "domination active",
        NextAction::array(0, new NextAction("handle domination", ACTION_INTERRUPT + 2), nullptr)));
    
    // Nether Wraith adds
    triggers.push_back(new TriggerNode(
        "nether wraith active",
        NextAction::array(0, new NextAction("attack nether wraith", ACTION_NORMAL + 6), nullptr)));
    
    // Arcane Torrent AoE
    triggers.push_back(new TriggerNode(
        "arcane torrent danger",
        NextAction::array(0, new NextAction("avoid arcane torrent", ACTION_MOVE + 2), nullptr)));
    
    // Enrage at 20%
    triggers.push_back(new TriggerNode(
        "pathaleon enraged",
        NextAction::array(0, new NextAction("handle pathaleon enrage", ACTION_EMERGENCY - 1), nullptr)));
    
    // Arcane Explosion (Heroic mode)
    triggers.push_back(new TriggerNode(
        "arcane explosion danger",
        NextAction::array(0, new NextAction("avoid arcane explosion", ACTION_MOVE + 3), nullptr)));
    
    // Mana Tap
    triggers.push_back(new TriggerNode(
        "mana tap active",
        NextAction::array(0, new NextAction("handle mana tap", ACTION_NORMAL + 1), nullptr)));
}

void MechanarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Block spellcasting during Raging Flames kiting
    multipliers.push_back(new MechanarMultiplier(botAI));
}