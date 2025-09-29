#include "TempestKeepStrategy.h"
#include "TempestKeepMultipliers.h"
#include "TempestKeepTriggerContext.h"
#include "TempestKeepActionContext.h"

void TempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Al'ar triggers with proper priorities (following ICC pattern)
    triggers.push_back(new TriggerNode(
        "alar flame quills",
        NextAction::array(0, new NextAction("alar flame quills", ACTION_EMERGENCY + 5), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "alar dive bomb",
        NextAction::array(0, new NextAction("alar dive bomb", ACTION_EMERGENCY + 4), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "alar flame patch",
        NextAction::array(0, new NextAction("alar flame patch", ACTION_EMERGENCY + 3), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "alar platform",
        NextAction::array(0, new NextAction("alar platform", ACTION_MOVE + 5), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "alar adds",
        NextAction::array(0, new NextAction("alar adds", ACTION_INTERRUPT + 2), nullptr)));

    // New Al'ar mechanics for 10/10 implementation
    triggers.push_back(new TriggerNode(
        "alar flame buffet",
        NextAction::array(0, new NextAction("alar flame buffet", ACTION_EMERGENCY + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "alar ember blast",
        NextAction::array(0, new NextAction("alar ember blast", ACTION_EMERGENCY + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "alar melt armor",
        NextAction::array(0, new NextAction("alar melt armor", ACTION_EMERGENCY + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "alar charge",
        NextAction::array(0, new NextAction("alar charge", ACTION_EMERGENCY + 4), nullptr)));

    // Void Reaver triggers
    triggers.push_back(new TriggerNode(
        "void reaver pounding",
        NextAction::array(0, new NextAction("void reaver pounding", ACTION_EMERGENCY + 4), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "void reaver arcane orb",
        NextAction::array(0, new NextAction("void reaver arcane orb", ACTION_NORMAL), nullptr)));

    triggers.push_back(new TriggerNode(
        "void reaver position",
        NextAction::array(0, new NextAction("void reaver position", ACTION_NORMAL), nullptr)));

    // Solarian triggers
    triggers.push_back(new TriggerNode(
        "solarian wrath",
        NextAction::array(0, new NextAction("solarian wrath", ACTION_EMERGENCY + 3), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "solarian blinding light",
        NextAction::array(0, new NextAction("solarian blinding light", ACTION_EMERGENCY + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "solarian portal",
        NextAction::array(0, new NextAction("solarian portal", ACTION_MOVE + 3), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "solarian adds",
        NextAction::array(0, new NextAction("solarian adds", ACTION_INTERRUPT + 2), nullptr)));

    // Kael'thas triggers
    triggers.push_back(new TriggerNode(
        "kaelthas advisors",
        NextAction::array(0, new NextAction("kaelthas advisors", ACTION_INTERRUPT + 3), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas weapons",
        NextAction::array(0, new NextAction("kaelthas weapons", ACTION_INTERRUPT + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas phoenix",
        NextAction::array(0, new NextAction("kaelthas phoenix", ACTION_INTERRUPT + 4), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas flamestrike",
        NextAction::array(0, new NextAction("kaelthas flamestrike", ACTION_EMERGENCY + 3), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas gravity lapse",
        NextAction::array(0, new NextAction("kaelthas gravity lapse", ACTION_EMERGENCY + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas pyroblast",
        NextAction::array(0, new NextAction("kaelthas pyroblast", ACTION_INTERRUPT + 5), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas mind control",
        NextAction::array(0, new NextAction("kaelthas mind control", ACTION_DISPEL + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "kaelthas nether vapor",
        NextAction::array(0, new NextAction("kaelthas nether vapor", ACTION_MOVE + 4), nullptr)));
        
    // Advisor specific triggers
    triggers.push_back(new TriggerNode(
        "thaladred fixate",
        NextAction::array(0, new NextAction("thaladred fixate", ACTION_EMERGENCY + 1), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "capernian conflagration",
        NextAction::array(0, new NextAction("capernian conflagration", ACTION_EMERGENCY + 1), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "telonicus remote toy",
        NextAction::array(0, new NextAction("telonicus remote toy", ACTION_DISPEL + 1), nullptr)));
}

void TempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AlarFlameQuillsMultiplier(botAI));
    multipliers.push_back(new AlarDiveBombMultiplier(botAI));
    multipliers.push_back(new AlarFlameBuffetMultiplier(botAI));
    multipliers.push_back(new AlarEmberBlastMultiplier(botAI));
    multipliers.push_back(new AlarMeltArmorMultiplier(botAI));
    multipliers.push_back(new AlarChargeMultiplier(botAI));
    multipliers.push_back(new VoidReaverPoundingMultiplier(botAI));
    multipliers.push_back(new VoidReaverPositionMultiplier(botAI));
    multipliers.push_back(new SolarianWrathMultiplier(botAI));
    multipliers.push_back(new SolarianBlindingLightMultiplier(botAI));
    multipliers.push_back(new KaelthasGravityLapseMultiplier(botAI));
}