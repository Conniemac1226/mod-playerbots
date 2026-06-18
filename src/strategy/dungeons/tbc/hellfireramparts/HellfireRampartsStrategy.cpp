#include "HellfireRampartsStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "HellfireRampartsMultipliers.h"

void TbcDungeonHRStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Boss: Watchkeeper Gargolmar
    // Hellfire Watchers join at 50% health - high priority target switch
    triggers.push_back(new TriggerNode("hellfire watcher active",
             NextAction::array(0, new NextAction("attack hellfire watcher", ACTION_EMERGENCY + 2), nullptr)));
    
    // Retaliation at 20% - ranged should back off
    triggers.push_back(new TriggerNode("gargolmar retaliation",
             NextAction::array(0, new NextAction("gargolmar retaliation", ACTION_MOVE + 2), nullptr)));
    
    // Boss: Omor the Unscarred - Keep essential survival, test combat blocking
    
    // Demonic Shield at 21% - ESSENTIAL for shield phase
    triggers.push_back(new TriggerNode("omor demonic shield",
             NextAction::array(0, new NextAction("omor demonic shield", ACTION_EMERGENCY), nullptr)));
    
    // Fiendish Hounds - exact WotLK pattern priority
    triggers.push_back(new TriggerNode("fiendish hound active",
             NextAction::array(0, new NextAction("attack fiendish hound", ACTION_RAID + 5), nullptr)));
    
    // Interrupt Shadow Bolt - ESSENTIAL to prevent damage
    triggers.push_back(new TriggerNode("omor shadow bolt cast",
             NextAction::array(0, new NextAction("interrupt omor shadow bolt", ACTION_INTERRUPT + 2), nullptr)));

    // HEROIC-READY SPREAD MECHANICS - IMMEDIATE RESPONSE TO TREACHERY CAST
    // CRITICAL: Highest priority when Omor starts casting - drop everything and spread
    triggers.push_back(new TriggerNode("omor treachery cast",
             NextAction::array(0, new NextAction("omor treachery spread", ACTION_EMERGENCY + 5), nullptr)));

    // DEBUFF AVOIDANCE - Stay away from players with Treacherous Aura
    triggers.push_back(new TriggerNode("omor debuff avoidance",
             NextAction::array(0, new NextAction("omor debuff avoidance", ACTION_EMERGENCY + 1), nullptr)));

    // CLEAR SPREAD - Remove spread when cast finishes to prevent permanent spreading
    triggers.push_back(new TriggerNode("omor clear spread",
             NextAction::array(0, new NextAction("omor clear spread", ACTION_NORMAL + 5), nullptr)));

    // Treacherous Aura - only the debuffed bot should move out
    triggers.push_back(new TriggerNode("omor treacherous aura",
             NextAction::array(0, new NextAction("omor treacherous aura", ACTION_EMERGENCY + 6), nullptr)));
    
    // Boss: Vazruden & Nazan
    // Avoid Liquid Fire patches - priority over other movement unless a real emergency
    triggers.push_back(new TriggerNode("liquid fire nearby",
             NextAction::array(0, new NextAction("avoid liquid fire", ACTION_MOVE + 5), nullptr)));
    
    // Avoid Cone of Fire
    triggers.push_back(new TriggerNode("nazan cone of fire",
             NextAction::array(0, new NextAction("avoid cone of fire", ACTION_MOVE + 4), nullptr)));
    
    // Bellowing Roar fear (Heroic)
    triggers.push_back(new TriggerNode("nazan bellowing roar",
             NextAction::array(0, new NextAction("nazan bellowing roar", ACTION_DISPEL + 2), nullptr)));
    
    // Attack Nazan first when landed
    triggers.push_back(new TriggerNode("nazan landed",
             NextAction::array(0, new NextAction("attack nazan first", ACTION_EMERGENCY + 3), nullptr)));
    
    // Attack Vazruden after Nazan dies
    triggers.push_back(new TriggerNode("vazruden alone",
             NextAction::array(0, new NextAction("attack vazruden", ACTION_NORMAL + 1), nullptr)));
}

void TbcDungeonHRStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation
    // RESEARCHED: Following HallsOfLightningStrategy.cpp:37-40 pattern
    multipliers.push_back(new OmorAddMultiplier(botAI));
}
