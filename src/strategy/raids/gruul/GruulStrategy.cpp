#include "GruulStrategy.h"
#include "GruulMultipliers.h"

void GruulStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul engaged",
        NextAction::array(0, new NextAction("gruul hurtful strike", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("gruul pre spread",
        NextAction::array(0, new NextAction("gruul pre spread", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("gruul ground slam",
        NextAction::array(0, new NextAction("gruul ground slam", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("gruul shatter",
        NextAction::array(0, new NextAction("gruul shatter position", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("gruul cave in",
        NextAction::array(0, new NextAction("gruul cave in", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("gruul reverberation",
        NextAction::array(0, new NextAction("gruul dispel", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode("gruul hurtful strike",
        NextAction::array(0, new NextAction("gruul hurtful strike", ACTION_MOVE + 3), nullptr)));
    triggers.push_back(new TriggerNode("gruul growth",
        NextAction::array(0, new NextAction("gruul tank swap", ACTION_HIGH + 2), nullptr)));
    
    // High King Maulgar and Council
    triggers.push_back(new TriggerNode("maulgar engaged",
        NextAction::array(0, new NextAction("maulgar position", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("maulgar council",
        NextAction::array(0, new NextAction("maulgar focus target", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("maulgar whirlwind",
        NextAction::array(0, new NextAction("maulgar whirlwind", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("maulgar arcing smash",
        NextAction::array(0, new NextAction("maulgar arcing smash", ACTION_MOVE + 4), nullptr)));
    triggers.push_back(new TriggerNode("maulgar berserker",
        NextAction::array(0, new NextAction("maulgar focus target", ACTION_HIGH + 3), nullptr)));
    
    // Krosh Firehand
    triggers.push_back(new TriggerNode("krosh spellshield",
        NextAction::array(0, new NextAction("krosh spellsteal", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(new TriggerNode("krosh blast wave",
        NextAction::array(0, new NextAction("krosh blast wave avoid", ACTION_MOVE + 5), nullptr)));
    
    // Kiggler the Crazed
    triggers.push_back(new TriggerNode("kiggler polymorph",
        NextAction::array(0, new NextAction("gruul dispel", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode("kiggler arcane explosion",
        NextAction::array(0, new NextAction("kiggler arcane explosion avoid", ACTION_MOVE + 4), nullptr)));
    
    // Olm the Summoner - Wild Fel Stalkers (HIGHEST PRIORITY - spawned adds)
    triggers.push_back(new TriggerNode("olm wild fel stalker",
        NextAction::array(0, new NextAction("olm wild fel stalker", ACTION_RAID + 5), nullptr)));
    
    // Blindeye the Seer
    triggers.push_back(new TriggerNode("blindeye heal",
        NextAction::array(0, new NextAction("blindeye interrupt", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode("blindeye shield",
        NextAction::array(0, new NextAction("gruul dispel", ACTION_DISPEL + 1), nullptr)));
}

void GruulStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // CRITICAL: Block DpsAssist when council present - prevents boss/add oscillation  
    // Following CLAUDE.md:678-702 pattern
    multipliers.push_back(new MaulgarAddMultiplier(botAI));
    
    // multipliers.push_back(new GruulMultiplier(botAI));
}
