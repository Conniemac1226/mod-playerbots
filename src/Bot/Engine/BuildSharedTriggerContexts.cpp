/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "Aq20TriggerContext.h"
#include "BTTriggerContext.h"
#include "BWLTriggerContext.h"
#include "ChatTriggerContext.h"
#include "EoETriggerContext.h"
#include "GruulTriggerContext.h"
#include "HyjalTriggerContext.h"
#include "ICCTriggerContext.h"
#include "KaraTriggerContext.h"
#include "MCTriggerContext.h"
#include "MagTriggerContext.h"
#include "NaxxTriggerContext.h"
#include "OSTriggerContext.h"
#include "OnyTriggerContext.h"
#include "OutlandWorldBossTriggerContext.h"
#include "RSTriggerContext.h"
#include "SSCTriggerContext.h"
#include "TKTriggerContext.h"
#include "TbcDungeonTriggerContext.h"
#include "Ai/Dungeon/DungeonAutoPullTriggerContext.h"
#include "TriggerContext.h"
#include "UldTriggerContext.h"
#include "VoATriggerContext.h"
#include "WorldPacketTriggerContext.h"
#include "WotlkDungeonTriggerContext.h"
#include "ZATriggerContext.h"
#include "../../../strategy/dungeons/tbc/arcatraz/ArcatrazTriggerContext.h"
#include "../../../strategy/dungeons/tbc/blackmorass/BlackMorassTriggerContext.h"
#include "../../../strategy/dungeons/tbc/bloodfurnace/BloodFurnaceTriggerContext.h"
#include "../../../strategy/dungeons/tbc/botanica/BotanicaTriggerContext.h"
#include "../../../strategy/dungeons/tbc/escapefromdurnholde/EscapeFromDurnholdeTriggerContext.h"
#include "../../../strategy/dungeons/tbc/hellfireramparts/HellfireRampartsTriggerContext.h"
#include "../../../strategy/dungeons/tbc/magistersterrace/MagistersTerraceTriggerContext.h"
#include "../../../strategy/dungeons/tbc/manatombs/ManaTombsTriggerContext.h"
#include "../../../strategy/dungeons/tbc/shadowlabyrinth/ShadowLabyrinthTriggerContext.h"
#include "../../../strategy/dungeons/tbc/shatteredhalls/ShatteredHallsTriggerContext.h"
#include "../../../strategy/dungeons/tbc/slavepens/SlavePensTriggerContext.h"
#include "../../../strategy/dungeons/tbc/steamvault/SteamvaultTriggerContext.h"
#include "../../../strategy/dungeons/tbc/underbog/UnderbogTriggerContext.h"

void AiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());
    triggerContexts.Add(new RaidAq20TriggerContext());
    triggerContexts.Add(new RaidMcTriggerContext());
    triggerContexts.Add(new RaidBwlTriggerContext());
    triggerContexts.Add(new RaidKarazhanTriggerContext());
    triggerContexts.Add(new RaidGruulsLairTriggerContext());
    triggerContexts.Add(new RaidMagtheridonTriggerContext());
    triggerContexts.Add(new RaidNaxxTriggerContext());
    triggerContexts.Add(new RaidSSCTriggerContext());
    triggerContexts.Add(new RaidTempestKeepTriggerContext());
    triggerContexts.Add(new RaidHyjalSummitTriggerContext());
    triggerContexts.Add(new RaidBlackTempleTriggerContext());
    triggerContexts.Add(new RaidZulAmanTriggerContext());
    triggerContexts.Add(new RaidOsTriggerContext());
    triggerContexts.Add(new RaidEoETriggerContext());
    triggerContexts.Add(new RaidVoATriggerContext());
    triggerContexts.Add(new RaidUlduarTriggerContext());
    triggerContexts.Add(new RaidOnyxiaTriggerContext());
    triggerContexts.Add(new RaidIccTriggerContext());
    triggerContexts.Add(new RaidRsTriggerContext());
    triggerContexts.Add(new TbcDungeonAuchenaiCryptsTriggerContext());
    triggerContexts.Add(new TbcDungeonSethekkHallsTriggerContext());
    triggerContexts.Add(new TbcDungeonMechanarTriggerContext());
    triggerContexts.Add(new RaidOutlandWorldBossTriggerContext());
    triggerContexts.Add(new DungeonAutoPullTriggerContext());
    triggerContexts.Add(new WotlkDungeonUKTriggerContext());
    triggerContexts.Add(new WotlkDungeonNexTriggerContext());
    triggerContexts.Add(new WotlkDungeonANTriggerContext());
    triggerContexts.Add(new WotlkDungeonOKTriggerContext());
    triggerContexts.Add(new WotlkDungeonDTKTriggerContext());
    triggerContexts.Add(new WotlkDungeonVHTriggerContext());
    triggerContexts.Add(new WotlkDungeonGDTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoLTriggerContext());
    triggerContexts.Add(new WotlkDungeonOccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUPTriggerContext());
    triggerContexts.Add(new WotlkDungeonCoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonFoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonPoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonToCTriggerContext());

    triggerContexts.Add(new EscapeFromDurnholdeTriggerContext());
    triggerContexts.Add(new BlackMorassTriggerContext());
    triggerContexts.Add(new MagistersTerraceTriggerContext());
    triggerContexts.Add(new SteamvaultTriggerContext());
    triggerContexts.Add(new HellfireRampartsTriggerContext());
    triggerContexts.Add(new BloodFurnaceTriggerContext());
    triggerContexts.Add(new SlavePensTriggerContext());
    triggerContexts.Add(new UnderbogTriggerContext());
    triggerContexts.Add(new ManaTombsTriggerContext());
    triggerContexts.Add(new ShatteredHallsTriggerContext());
    triggerContexts.Add(new ShadowLabyrinthTriggerContext());
    triggerContexts.Add(new ArcatrazTriggerContext());
    triggerContexts.Add(new BotanicaTriggerContext());
}
