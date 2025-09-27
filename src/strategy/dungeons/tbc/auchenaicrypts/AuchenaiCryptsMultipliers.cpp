#include "AuchenaiCryptsMultipliers.h"
#include "AuchenaiCryptsTriggers.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"

float ShirrakFocusFireMultiplier::GetValue(Action* action)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // Critical priority for Focus Fire avoidance
    if (action->getName() == "shirrak focus fire avoid")
    {
        if (TbcDungeon::FindClosestNpcByEntry(botAI, bot, NPC_FOCUS_FIRE, 60.0f))
            return 5.0f;
    }

    return 1.0f;
}
