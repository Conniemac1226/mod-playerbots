/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OUTLANDWORLDBOSSTRIGGERCONTEXT_H
#define _PLAYERBOT_OUTLANDWORLDBOSSTRIGGERCONTEXT_H

#include "BossAuraTriggers.h"
#include "NamedObjectContext.h"
#include "OutlandWorldBossStrategy.h"

class RaidOutlandWorldBossTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidOutlandWorldBossTriggerContext()
    {
        creators["doomwalker nature resistance"] = &RaidOutlandWorldBossTriggerContext::doomwalker_nature_resistance;
        creators["doomwalker chain lightning"] = &RaidOutlandWorldBossTriggerContext::doomwalker_chain_lightning;
        creators["doom lord kazzak shadow resistance"] =
            &RaidOutlandWorldBossTriggerContext::doom_lord_kazzak_shadow_resistance;
        creators["doom lord kazzak mark of kazzak"] =
            &RaidOutlandWorldBossTriggerContext::doom_lord_kazzak_mark_of_kazzak;
        creators["doom lord kazzak twisted reflection"] =
            &RaidOutlandWorldBossTriggerContext::doom_lord_kazzak_twisted_reflection;
    }

private:
    static Trigger* doomwalker_nature_resistance(PlayerbotAI* botAI)
    {
        return new BossNatureResistanceTrigger(botAI, "doomwalker");
    }

    static Trigger* doomwalker_chain_lightning(PlayerbotAI* botAI)
    {
        return new DoomwalkerChainLightningTrigger(botAI);
    }

    static Trigger* doom_lord_kazzak_shadow_resistance(PlayerbotAI* botAI)
    {
        return new BossShadowResistanceTrigger(botAI, "doom lord kazzak");
    }

    static Trigger* doom_lord_kazzak_mark_of_kazzak(PlayerbotAI* botAI)
    {
        return new DoomLordKazzakMarkOfKazzakTrigger(botAI);
    }

    static Trigger* doom_lord_kazzak_twisted_reflection(PlayerbotAI* botAI)
    {
        return new DoomLordKazzakTwistedReflectionTrigger(botAI);
    }
};

#endif
