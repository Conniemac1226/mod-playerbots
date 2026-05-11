#include "MagistersTerraceMultipliers.h"
#include "MagistersTerraceActions.h"
#include "MagistersTerraceTriggers.h"
#include "Unit.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float MagistersTerraceMultiplier::GetValue(Action* action)
{
    // No multipliers needed - using proper priority values in strategy
    // Following the pattern from fixed Sethekk Halls and Auchenai Crypts
    return 1.0f;
}

float VexallusPureEnergyMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for Pure Energy add present  
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_PURE_ENERGY)
        {
            return 0.0f; // Block DpsAssist when Pure Energy present
        }
    }
    return 1.0f;
}

float SelinFelCrystalMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for active Fel Crystal present
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_FEL_CRYSTAL)
        {
            // Check if crystal is active (not just decorative)
            bool isActive = !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
                           unit->HasUnitState(UNIT_STATE_CASTING) ||
                           unit->HasAura(SPELL_MANA_RAGE);
            
            if (isActive)
            {
                return 0.0f; // Block DpsAssist when active Fel Crystal present
            }
        }
    }
    return 1.0f;
}

float KaelthasPhoenixesAndEggsMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    return MagistersTerraceHelpers::SelectKaelthasPhoenixTarget(bot, botAI, boss) ? 0.0f : 1.0f;
}

float DelrissaAddMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 120.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    return MagistersTerraceHelpers::GetDelrissaHelpersCached(botAI, bot).empty() ? 1.0f : 0.0f;
}
