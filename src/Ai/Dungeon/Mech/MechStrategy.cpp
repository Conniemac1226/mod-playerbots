#include "MechStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "MechMultipliers.h"
#include "MechTriggers.h"

void TbcDungeonMechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Mechano-Lord Capacitus (preserved from the former custom strategy).
    triggers.push_back(new TriggerNode("reflective shield active", {
        NextAction("handle reflective shield", ACTION_INTERRUPT + 1) }));
    triggers.push_back(new TriggerNode("polarity shift active", {
        NextAction("handle polarity shift", ACTION_EMERGENCY) }));
    triggers.push_back(new TriggerNode("nether charge active", {
        NextAction("attack nether charge", ACTION_NORMAL + 5) }));
    triggers.push_back(new TriggerNode("capacitus engaged", {
        NextAction("capacitus position", ACTION_MOVE + 1) }));

    triggers.push_back(new TriggerNode("sepethrea kite flame", {
        NextAction("sepethrea kite flame", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("sepethrea avoid flame", {
        NextAction("sepethrea avoid flame", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("sepethrea trail", {
        NextAction("sepethrea avoid trail", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("sepethrea focus boss", {
        NextAction("sepethrea focus boss", ACTION_RAID + 2) }));

    // Pathaleon the Calculator (preserved from the former custom strategy).
    triggers.push_back(new TriggerNode("domination active", {
        NextAction("handle domination", ACTION_INTERRUPT + 2) }));
    triggers.push_back(new TriggerNode("nether wraith active", {
        NextAction("attack nether wraith", ACTION_NORMAL + 6) }));
    triggers.push_back(new TriggerNode("arcane torrent danger", {
        NextAction("avoid arcane torrent", ACTION_MOVE + 2) }));
    triggers.push_back(new TriggerNode("pathaleon enraged", {
        NextAction("handle pathaleon enrage", ACTION_EMERGENCY - 1) }));
    triggers.push_back(new TriggerNode("arcane explosion danger", {
        NextAction("avoid arcane explosion", ACTION_MOVE + 3) }));
    triggers.push_back(new TriggerNode("mana tap active", {
        NextAction("handle mana tap", ACTION_NORMAL + 1) }));
}

void TbcDungeonMechanarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SepethreaKiteFlameMultiplier(botAI));
    multipliers.push_back(new SepethreaFocusBossMultiplier(botAI));
}
