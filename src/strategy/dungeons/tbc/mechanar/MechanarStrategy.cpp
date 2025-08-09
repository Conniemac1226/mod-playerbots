#include "MechanarStrategy.h"
#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

MechanarStrategy::MechanarStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}

void MechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // ========== MECHANO LORD CAPACITUS ==========
    
    // RESEARCHED: boss_mechano_lord_capacitus.cpp:89-96 - Reflective shields (Normal)
    triggers.push_back(new TriggerNode(
        "reflective shield active",
        NextAction::array(0, new NextAction("handle reflective shield", ACTION_INTERRUPT + 1), nullptr)));
    
    // RESEARCHED: boss_mechano_lord_capacitus.cpp:78-82 - Polarity Shift (Heroic)
    triggers.push_back(new TriggerNode(
        "polarity shift active",
        NextAction::array(0, new NextAction("handle polarity shift", ACTION_EMERGENCY), nullptr)));
    
    // RESEARCHED: boss_mechano_lord_capacitus.cpp:64-70 - Nether Charges
    triggers.push_back(new TriggerNode(
        "nether charge active",
        NextAction::array(0, new NextAction("attack nether charge", ACTION_NORMAL + 5), nullptr)));
    
    // General positioning for Capacitus
    triggers.push_back(new TriggerNode(
        "capacitus engaged",
        NextAction::array(0, new NextAction("capacitus position", ACTION_MOVE + 1), nullptr)));
    
    // ========== NETHERMANCER SEPETHREA ==========
    
    // RESEARCHED: boss_nethermancer_sepethrea.cpp:146-157 - Raging Flames fixate
    triggers.push_back(new TriggerNode(
        "raging flames active",
        NextAction::array(0, new NextAction("flee raging flames", ACTION_EMERGENCY), nullptr)));
    
    // RESEARCHED: boss_nethermancer_sepethrea.cpp:72 - Dragon's Breath frontal cone
    triggers.push_back(new TriggerNode(
        "dragons breath danger",
        NextAction::array(0, new NextAction("avoid dragons breath", ACTION_MOVE + 4), nullptr)));
    
    // RESEARCHED: boss_nethermancer_sepethrea.cpp:153 - Inferno AoE
    triggers.push_back(new TriggerNode(
        "inferno danger",
        NextAction::array(0, new NextAction("avoid inferno", ACTION_MOVE + 3), nullptr)));
    
    // RESEARCHED: boss_nethermancer_sepethrea.cpp:63-68 - Arcane Blast threat reduction
    triggers.push_back(new TriggerNode(
        "sepethrea engaged",
        NextAction::array(0, new NextAction("handle arcane blast", ACTION_NORMAL + 2), nullptr)));
    
    // ========== PATHALEON THE CALCULATOR ==========
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:117-122 - Mind Control
    triggers.push_back(new TriggerNode(
        "domination active",
        NextAction::array(0, new NextAction("handle domination", ACTION_INTERRUPT + 2), nullptr)));
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:96-101 - Nether Wraith adds
    triggers.push_back(new TriggerNode(
        "nether wraith active",
        NextAction::array(0, new NextAction("attack nether wraith", ACTION_NORMAL + 6), nullptr)));
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:113-115 - Arcane Torrent AoE
    triggers.push_back(new TriggerNode(
        "arcane torrent danger",
        NextAction::array(0, new NextAction("avoid arcane torrent", ACTION_MOVE + 2), nullptr)));
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:84-90 - Enrage at 20%
    triggers.push_back(new TriggerNode(
        "pathaleon enraged",
        NextAction::array(0, new NextAction("handle pathaleon enrage", ACTION_EMERGENCY - 1), nullptr)));
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:131-135 - Arcane Explosion (Heroic)
    triggers.push_back(new TriggerNode(
        "arcane explosion danger",
        NextAction::array(0, new NextAction("avoid arcane explosion", ACTION_MOVE + 3), nullptr)));
    
    // RESEARCHED: boss_pathaleon_the_calculator.cpp:104-109 - Mana Tap
    triggers.push_back(new TriggerNode(
        "mana tap active",
        NextAction::array(0, new NextAction("handle mana tap", ACTION_NORMAL + 1), nullptr)));
}