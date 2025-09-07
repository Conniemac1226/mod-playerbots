#include "ShadowLabyrinthStrategy.h"
#include "ShadowLabyrinthMultipliers.h"

void ShadowLabyrinthStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Ambassador Hellmaw
    triggers.push_back(new TriggerNode("hellmaw corrosive acid",
        NextAction::array(0, new NextAction("avoid corrosive acid", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("hellmaw fear",
        NextAction::array(0, new NextAction("hellmaw fear react", ACTION_DISPEL + 2), nullptr)));
    
    // Blackheart the Inciter
    triggers.push_back(new TriggerNode("blackheart incite chaos",
        NextAction::array(0, new NextAction("incite chaos react", ACTION_RAID + 3), nullptr)));
    
    triggers.push_back(new TriggerNode("blackheart war stomp",
        NextAction::array(0, new NextAction("avoid war stomp", ACTION_MOVE + 5), nullptr)));
        
    triggers.push_back(new TriggerNode("blackheart charge",
        NextAction::array(0, new NextAction("blackheart charge react", ACTION_MOVE + 4), nullptr)));
    
    // Grandmaster Vorpil
    triggers.push_back(new TriggerNode("vorpil void traveler",
        NextAction::array(0, new NextAction("void traveler priority", ACTION_EMERGENCY), nullptr)));
    
    triggers.push_back(new TriggerNode("vorpil rain of fire",
        NextAction::array(0, new NextAction("move from rain of fire", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("vorpil draw shadows",
        NextAction::array(0, new NextAction("draw shadows react", ACTION_MOVE + 6), nullptr)));
    
    // Murmur
    triggers.push_back(new TriggerNode("murmur sonic boom",
        NextAction::array(0, new NextAction("murmur sonic boom", ACTION_EMERGENCY), nullptr)));
    
    triggers.push_back(new TriggerNode("murmur resonance",
        NextAction::array(0, new NextAction("murmur resonance", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("murmur magnetic pull",
        NextAction::array(0, new NextAction("murmur magnetic pull", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("murmur thundering storm",
        NextAction::array(0, new NextAction("murmur thundering storm", ACTION_MOVE + 4), nullptr)));
}

void ShadowLabyrinthStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new HellmawMultiplier(botAI));
    multipliers.push_back(new BlackheartMultiplier(botAI));
    multipliers.push_back(new VorpilMultiplier(botAI));
    multipliers.push_back(new MurmurMultiplier(botAI));
    // CRITICAL: Block DpsAssist when Void Travelers present - prevents boss/add oscillation
    multipliers.push_back(new VorpilVoidTravelerMultiplier(botAI));
}