/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "ActionContext.h"
#include "ChatActionContext.h"
#include "WorldPacketActionContext.h"
#include "Aq20ActionContext.h"
#include "MCActionContext.h"
#include "BWLActionContext.h"
#include "KaraActionContext.h"
#include "GruulActionContext.h"
#include "NaxxActionContext.h"
#include "MagActionContext.h"
#include "SSCActionContext.h"
#include "TKActionContext.h"
#include "HyjalActionContext.h"
#include "BTActionContext.h"
#include "ZAActionContext.h"
#include "OSActionContext.h"
#include "EoEActionContext.h"
#include "VoAActionContext.h"
#include "UldActionContext.h"
#include "OnyActionContext.h"
#include "ICCActionContext.h"
#include "RSActionContext.h"
#include "OutlandWorldBossActionContext.h"
#include "Ai/Dungeon/DungeonAutoPullActionContext.h"
#include "Ai/Dungeon/TbcDungeonActionContext.h"
#include "Ai/Dungeon/WotlkDungeonActionContext.h"
#include "MechActionContext.h"
#include "SethActionContext.h"
#include "../../../strategy/raids/blacktemple/BlackTempleActionContext.h"
#include "../../../strategy/dungeons/tbc/blackmorass/BlackMorassActionContext.h"
#include "../../../strategy/dungeons/tbc/escapefromdurnholde/EscapeFromDurnholdeActionContext.h"
#include "../../../strategy/dungeons/tbc/magistersterrace/MagistersTerraceActionContext.h"
#include "../../../strategy/dungeons/tbc/steamvault/SteamvaultActionContext.h"
#include "../../../strategy/dungeons/tbc/hellfireramparts/HellfireRampartsActionContext.h"
#include "../../../strategy/dungeons/tbc/bloodfurnace/BloodFurnaceActionContext.h"
#include "../../../strategy/dungeons/tbc/slavepens/SlavePensActionContext.h"
#include "../../../strategy/dungeons/tbc/underbog/UnderbogActionContext.h"
#include "../../../strategy/dungeons/tbc/manatombs/ManaTombsActionContext.h"
#include "../../../strategy/dungeons/tbc/shatteredhalls/ShatteredHallsActionContext.h"
#include "../../../strategy/dungeons/tbc/shadowlabyrinth/ShadowLabyrinthActionContext.h"
#include "../../../strategy/dungeons/tbc/arcatraz/ArcatrazActionContext.h"
#include "../../../strategy/dungeons/tbc/botanica/BotanicaActionContext.h"

void AiObjectContext::BuildSharedActionContexts(SharedNamedObjectContextList<Action>& actionContexts)
{
    actionContexts.Add(new ActionContext());
    actionContexts.Add(new ChatActionContext());
    actionContexts.Add(new WorldPacketActionContext());
    actionContexts.Add(new RaidAq20ActionContext());
    actionContexts.Add(new RaidMcActionContext());
    actionContexts.Add(new RaidBwlActionContext());
    actionContexts.Add(new RaidKarazhanActionContext());
    actionContexts.Add(new RaidGruulsLairActionContext());
    actionContexts.Add(new RaidMagtheridonActionContext());
    actionContexts.Add(new RaidSSCActionContext());
    actionContexts.Add(new RaidTempestKeepActionContext());
    actionContexts.Add(new RaidHyjalSummitActionContext());
    actionContexts.Add(new RaidBlackTempleActionContext());
    actionContexts.Add(new RaidZulAmanActionContext());
    actionContexts.Add(new RaidNaxxActionContext());
    actionContexts.Add(new RaidOsActionContext());
    actionContexts.Add(new RaidEoEActionContext());
    actionContexts.Add(new RaidVoAActionContext());
    actionContexts.Add(new RaidUlduarActionContext());
    actionContexts.Add(new RaidOnyxiaActionContext());
    actionContexts.Add(new RaidIccActionContext());
    actionContexts.Add(new RaidRsActionContext());
    actionContexts.Add(new RaidOutlandWorldBossActionContext());
    actionContexts.Add(new DungeonAutoPullActionContext());
    actionContexts.Add(new TbcDungeonAuchenaiCryptsActionContext());
    actionContexts.Add(new TbcDungeonSethekkHallsActionContext());
    actionContexts.Add(new TbcDungeonMechanarActionContext());
    actionContexts.Add(new WotlkDungeonUKActionContext());
    actionContexts.Add(new WotlkDungeonNexActionContext());
    actionContexts.Add(new WotlkDungeonANActionContext());
    actionContexts.Add(new WotlkDungeonOKActionContext());
    actionContexts.Add(new WotlkDungeonDTKActionContext());
    actionContexts.Add(new WotlkDungeonVHActionContext());
    actionContexts.Add(new WotlkDungeonGDActionContext());
    actionContexts.Add(new WotlkDungeonHoSActionContext());
    actionContexts.Add(new WotlkDungeonHoLActionContext());
    actionContexts.Add(new WotlkDungeonOccActionContext());
    actionContexts.Add(new WotlkDungeonUPActionContext());
    actionContexts.Add(new WotlkDungeonCoSActionContext());
    actionContexts.Add(new WotlkDungeonFoSActionContext());
    actionContexts.Add(new WotlkDungeonPoSActionContext());
    actionContexts.Add(new WotlkDungeonToCActionContext());

    // Custom / Unique
    actionContexts.Add(new RaidBtActionContext());
    actionContexts.Add(new EscapeFromDurnholdeActionContext());
    actionContexts.Add(new BlackMorassActionContext());
    actionContexts.Add(new MagistersTerraceActionContext());
    actionContexts.Add(new SteamvaultActionContext());
    actionContexts.Add(new HellfireRampartsActionContext());
    actionContexts.Add(new BloodFurnaceActionContext());
    actionContexts.Add(new SlavePensActionContext());
    actionContexts.Add(new UnderbogActionContext());
    actionContexts.Add(new ManaTombsActionContext());
    actionContexts.Add(new ShatteredHallsActionContext());
    actionContexts.Add(new ShadowLabyrinthActionContext());
    actionContexts.Add(new ArcatrazActionContext());
    actionContexts.Add(new BotanicaActionContext());
}
