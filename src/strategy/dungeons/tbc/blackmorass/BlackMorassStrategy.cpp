#include "BlackMorassStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "BlackMorassActions.h"
#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

BlackMorassStrategy::BlackMorassStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}

void BlackMorassStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // ===== PORTAL/ADD MANAGEMENT (HIGHEST PRIORITY) =====
    triggers.push_back(new TriggerNode(
        "portal add active",
        NextAction::array(0, new NextAction("attack portal add", ACTION_EMERGENCY), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "medivh needs protection",
        NextAction::array(0, new NextAction("protect medivh", ACTION_MOVE + 3), nullptr)));

    // ===== AEONUS TRIGGERS =====
    triggers.push_back(new TriggerNode(
        "aeonus cleave danger",
        NextAction::array(0, new NextAction("aeonus avoid cleave", ACTION_EMERGENCY - 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "aeonus engaged",
        NextAction::array(0, new NextAction("aeonus position", ACTION_MOVE + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "sand breath danger",
        NextAction::array(0, new NextAction("avoid sand breath", ACTION_EMERGENCY), nullptr)));

    // ===== CHRONO LORD DEJA TRIGGERS =====
    triggers.push_back(new TriggerNode(
        "time lapse danger", 
        NextAction::array(0, new NextAction("avoid time lapse", ACTION_EMERGENCY - 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "arcane discharge danger",
        NextAction::array(0, new NextAction("avoid arcane discharge", ACTION_EMERGENCY - 1), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "attraction active",
        NextAction::array(0, new NextAction("handle attraction", ACTION_EMERGENCY - 2), nullptr)));

    // ===== TEMPORUS TRIGGERS =====
    triggers.push_back(new TriggerNode(
        "wing buffet danger",
        NextAction::array(0, new NextAction("avoid wing buffet", ACTION_EMERGENCY - 1), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "mortal wound active",
        NextAction::array(0, new NextAction("heal mortal wound", ACTION_CRITICAL_HEAL + 2), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "temporus reflect active",
        NextAction::array(0, new NextAction("stop casting reflect", ACTION_INTERRUPT + 1), nullptr)));
        
    // RESEARCHED: boss_temporus.cpp:54 - HASTEN buff needs dispel
    triggers.push_back(new TriggerNode(
        "temporus hasten active",
        NextAction::array(0, new NextAction("dispel hasten", ACTION_INTERRUPT), nullptr)));
        
    // RESEARCHED: boss_chrono_lord_deja.cpp:58 - ARCANE_BLAST needs interrupt
    triggers.push_back(new TriggerNode(
        "deja arcane blast casting",
        NextAction::array(0, new NextAction("interrupt arcane blast", ACTION_INTERRUPT + 2), nullptr)));
}

float PortalAddMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && IsBlackMorassPortalAdd(unit->GetEntry()))
        {
            return 0.0f; // Block DpsAssist when any Portal Add present
        }
    }
    return 1.0f;
}

void BlackMorassStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // CRITICAL: Block DpsAssist when portal adds present - prevents boss/add oscillation
    // CLAUDE.MD COMPLIANCE: This multiplier is MANDATORY for add encounters
    multipliers.push_back(new PortalAddMultiplier(botAI));
}
