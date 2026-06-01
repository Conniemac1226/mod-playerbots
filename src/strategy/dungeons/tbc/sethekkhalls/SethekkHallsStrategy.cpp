#include "SethekkHallsStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "SethekkHallsMultipliers.h"

void TbcDungeonSHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Trash: Sethekk Spirit - Flee from ghost spawned when Sethekk Prophets die
    triggers.push_back(new TriggerNode("sethekk spirit nearby",
             NextAction::array(0, new NextAction("flee sethekk spirit", ACTION_EMERGENCY + 3), nullptr)));

    // Darkweaver Syth: keep non-tanks near melee range between elemental waves.
    triggers.push_back(new TriggerNode("syth no elementals",
             NextAction::array(0, new NextAction("stack for syth", ACTION_MOVE + 2), nullptr)));

    // Trash: keep tank protected from fear without changing global class behavior.
    triggers.push_back(new TriggerNode("sethekk anti fear needed",
             NextAction::array(0,
                new NextAction("sethekk fear ward tank", ACTION_HIGH + 4),
                new NextAction("sethekk tremor totem", ACTION_HIGH + 3),
                nullptr)));
    
    // Trash: Time-Lost Controller - ICC Pattern: Skull mark Charming Totem
    // RESEARCHED: RaidIccActions.cpp:1276-1297 (UpdateSkullMarker pattern)
    triggers.push_back(new TriggerNode("charming totem spawned",
             NextAction::array(0,
                new NextAction("mark charming totem", ACTION_EMERGENCY + 2),
                new NextAction("continue fight with charmed ally", ACTION_MOVE + 1),
                nullptr)));

    triggers.push_back(new TriggerNode("time lost controller casting totem",
             NextAction::array(0, new NextAction("interrupt controller", ACTION_INTERRUPT + 1), nullptr)));
    
    triggers.push_back(new TriggerNode("ikiss arcane explosion cast",
             NextAction::array(0, new NextAction("ikiss move away", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("ikiss arcane explosion ended",
             NextAction::array(0, new NextAction("ikiss return position", ACTION_MOVE + 3), nullptr)));

    triggers.push_back(new TriggerNode("ikiss tank pillar position needed",
             NextAction::array(0, new NextAction("ikiss tank pillar position", ACTION_MOVE + 2), nullptr)));

    // Boss: Anzu - Priority add targeting (banish phases at 66% and 33%)
    triggers.push_back(new TriggerNode("brood of anzu nearby",
             NextAction::array(0, new NextAction("attack brood of anzu", ACTION_NORMAL + 5), nullptr)));

}

void TbcDungeonSHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // ICC Pattern: Skull marking replaces need for CharmingTotemMultiplier
    // CRITICAL: Block DpsAssist when Brood adds present - prevents boss/add oscillation
    multipliers.push_back(new BroodOfAnzuMultiplier(botAI));
}
