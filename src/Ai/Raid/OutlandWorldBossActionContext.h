/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OUTLANDWORLDBOSSACTIONCONTEXT_H
#define _PLAYERBOT_OUTLANDWORLDBOSSACTIONCONTEXT_H

#include "BossAuraActions.h"
#include "NamedObjectContext.h"
#include "OutlandWorldBossStrategy.h"

class RaidOutlandWorldBossActionContext : public NamedObjectContext<Action>
{
public:
    RaidOutlandWorldBossActionContext()
    {
        creators["doomwalker nature resistance"] = &RaidOutlandWorldBossActionContext::doomwalker_nature_resistance;
        creators["doomwalker spread for chain lightning"] =
            &RaidOutlandWorldBossActionContext::doomwalker_spread_for_chain_lightning;
        creators["doomwalker move away from earthquake"] =
            &RaidOutlandWorldBossActionContext::doomwalker_move_away_from_earthquake;
        creators["doom lord kazzak shadow resistance"] =
            &RaidOutlandWorldBossActionContext::doom_lord_kazzak_shadow_resistance;
        creators["doom lord kazzak move away from mark of kazzak"] =
            &RaidOutlandWorldBossActionContext::doom_lord_kazzak_move_away_from_mark_of_kazzak;
        creators["doom lord kazzak move away during twisted reflection"] =
            &RaidOutlandWorldBossActionContext::doom_lord_kazzak_move_away_during_twisted_reflection;
    }

private:
    static Action* doomwalker_nature_resistance(PlayerbotAI* botAI)
    {
        return new BossNatureResistanceAction(botAI, "doomwalker");
    }

    static Action* doomwalker_spread_for_chain_lightning(PlayerbotAI* botAI)
    {
        return new DoomwalkerChainLightningSpreadAction(botAI);
    }

    static Action* doomwalker_move_away_from_earthquake(PlayerbotAI* botAI)
    {
        return new DoomwalkerEarthquakeMoveAwayAction(botAI);
    }

    static Action* doom_lord_kazzak_shadow_resistance(PlayerbotAI* botAI)
    {
        return new BossShadowResistanceAction(botAI, "doom lord kazzak");
    }

    static Action* doom_lord_kazzak_move_away_from_mark_of_kazzak(PlayerbotAI* botAI)
    {
        return new DoomLordKazzakMoveAwayFromMarkAction(botAI);
    }

    static Action* doom_lord_kazzak_move_away_during_twisted_reflection(PlayerbotAI* botAI)
    {
        return new DoomLordKazzakMoveAwayDuringTwistedReflectionAction(botAI);
    }
};

#endif
