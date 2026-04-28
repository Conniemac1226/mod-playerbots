#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("gruuls lair auto pull ready", {
        NextAction("gruuls lair auto pull trash", ACTION_MOVE + 3) }));

    triggers.push_back(new TriggerNode("ready check finished", {
        NextAction("gruuls lair mark boss pull ready", ACTION_RAID + 10) }));

    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar is main tank", {
        NextAction("high king maulgar main tank attack maulgar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is first assist tank", {
        NextAction("high king maulgar first assist tank attack olm", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is second assist tank", {
        NextAction("high king maulgar second assist tank attack blindeye", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is mage tank", {
        NextAction("high king maulgar mage tank attack krosh", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is moonkin tank", {
        NextAction("high king maulgar moonkin tank attack kiggler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar determining kill order", {
        NextAction("high king maulgar assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar healer in danger", {
        NextAction("high king maulgar healer find safe position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar blindeye casting", {
        NextAction("high king maulgar interrupt blindeye", ACTION_INTERRUPT + 2) }));

    triggers.push_back(new TriggerNode("high king maulgar boss channeling whirlwind", {
        NextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("high king maulgar wild felstalker spawned", {
        NextAction("high king maulgar banish felstalker", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high king maulgar pulling olm and blindeye", {
        NextAction("high king maulgar misdirect olm and blindeye", ACTION_RAID + 2) }));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by tanks", {
        NextAction("gruul the dragonkiller tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by ranged", {
        NextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller cave in", {
        NextAction("avoid aoe", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller incoming shatter", {
        NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6) }));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableArcaneShotOnKroshMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableMageTankAOEMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerMainTankMovementMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerDpsWaitMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerGroundSlamMultiplier(botAI));
}
