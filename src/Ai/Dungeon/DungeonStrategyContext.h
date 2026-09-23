/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUNGEONSTRATEGYCONTEXT_H
#define PLAYERBOTS_DUNGEONSTRATEGYCONTEXT_H

#include "ACStrategy.h"
#include "AKStrategy.h"
#include "ANStrategy.h"
#include "CoSStrategy.h"
#include "DTKStrategy.h"
#include "FoSStrategy.h"
#include "GDStrategy.h"
#include "HoLStrategy.h"
#include "HoSStrategy.h"
#include "MechStrategy.h"
#include "MgTStrategy.h"
#include "NexStrategy.h"
#include "OCStrategy.h"
#include "PoSStrategy.h"
#include "RampStrategy.h"
#include "SethStrategy.h"
#include "Strategy.h"
#include "TOCStrategy.h"
#include "UBStrategy.h"
#include "UKStrategy.h"
#include "UPStrategy.h"
#include "VHStrategy.h"
#include "../../strategy/dungeons/tbc/arcatraz/ArcatrazStrategy.h"
#include "../../strategy/dungeons/tbc/blackmorass/BlackMorassStrategy.h"
#include "../../strategy/dungeons/tbc/bloodfurnace/BloodFurnaceStrategy.h"
#include "../../strategy/dungeons/tbc/botanica/BotanicaStrategy.h"
#include "../../strategy/dungeons/tbc/escapefromdurnholde/EscapeFromDurnholdeStrategy.h"
#include "../../strategy/dungeons/tbc/manatombs/ManaTombsStrategy.h"
#include "../../strategy/dungeons/tbc/shadowlabyrinth/ShadowLabyrinthStrategy.h"
#include "../../strategy/dungeons/tbc/shatteredhalls/ShatteredHallsStrategy.h"
#include "../../strategy/dungeons/tbc/slavepens/SlavePensStrategy.h"
#include "../../strategy/dungeons/tbc/steamvault/SteamvaultStrategy.h"

class DungeonStrategyContext : public NamedObjectContext<Strategy>
{
public:
    DungeonStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        // Vanilla
        // ...

        // Burning Crusade
        creators["tbc-ac"] = &DungeonStrategyContext::tbc_ac;           // Auchindoun: Auchenai Crypts
        creators["tbc-seth"] = &DungeonStrategyContext::tbc_seth;       // Auchindoun: Sethekk Halls
        creators["tbc-mech"] = &DungeonStrategyContext::tbc_mech;       // Tempest Keep: The Mechanar
        creators["tbc-ub"] = &DungeonStrategyContext::tbc_ub;           // Coilfang Reservoir: The Underbog
        creators["tbc-mgt"] = &DungeonStrategyContext::tbc_mgt;         // Magisters' Terrace
        creators["tbc-ramp"] = &DungeonStrategyContext::tbc_ramp;       // Hellfire Citadel: Hellfire Ramparts
        creators["tbc-efd"] = &DungeonStrategyContext::tbc_efd;         // Escape from Durnholde
        creators["tbc-bm"] = &DungeonStrategyContext::tbc_bm;           // Black Morass
        creators["tbc-sv"] = &DungeonStrategyContext::tbc_sv;           // The Steamvault
        creators["tbc-bf"] = &DungeonStrategyContext::tbc_bf;           // The Blood Furnace
        creators["tbc-sp"] = &DungeonStrategyContext::tbc_sp;           // The Slave Pens
        creators["tbc-mato"] = &DungeonStrategyContext::tbc_mato;       // Mana-Tombs
        creators["tbc-shh"] = &DungeonStrategyContext::tbc_shh;         // The Shattered Halls
        creators["tbc-sl"] = &DungeonStrategyContext::tbc_sl;           // Shadow Labyrinth
        creators["tbc-arc"] = &DungeonStrategyContext::tbc_arc;         // The Arcatraz
        creators["tbc-bot"] = &DungeonStrategyContext::tbc_bot;         // The Botanica

        // Wrath of the Lich King
        creators["wotlk-uk"] = &DungeonStrategyContext::wotlk_uk;       // Utgarde Keep
        creators["wotlk-nex"] = &DungeonStrategyContext::wotlk_nex;     // The Nexus
        creators["wotlk-an"] = &DungeonStrategyContext::wotlk_an;       // Azjol-Nerub
        creators["wotlk-ok"] = &DungeonStrategyContext::wotlk_ok;       // Ahn'kahet: The Old Kingdom
        creators["wotlk-dtk"] = &DungeonStrategyContext::wotlk_dtk;     // Drak'Tharon Keep
        creators["wotlk-vh"] = &DungeonStrategyContext::wotlk_vh;       // The Violet Hold
        creators["wotlk-gd"] = &DungeonStrategyContext::wotlk_gd;       // Gundrak
        creators["wotlk-hos"] = &DungeonStrategyContext::wotlk_hos;     // Halls of Stone
        creators["wotlk-hol"] = &DungeonStrategyContext::wotlk_hol;     // Halls of Lightning
        creators["wotlk-occ"] = &DungeonStrategyContext::wotlk_occ;     // The Oculus
        creators["wotlk-up"] = &DungeonStrategyContext::wotlk_up;       // Utgarde Pinnacle
        creators["wotlk-cos"] = &DungeonStrategyContext::wotlk_cos;     // The Culling of Stratholme
        creators["wotlk-toc"] = &DungeonStrategyContext::wotlk_toc;     // Trial of the Champion
        creators["wotlk-pos"] = &DungeonStrategyContext::wotlk_pos;     // Pit of Saron
        creators["wotlk-fos"] = &DungeonStrategyContext::wotlk_fos;     // The Forge of Souls
    }

private:
    static Strategy* tbc_ac(PlayerbotAI* botAI) { return new TbcDungeonAuchenaiCryptsStrategy(botAI); }
    static Strategy* tbc_seth(PlayerbotAI* botAI) { return new TbcDungeonSethekkHallsStrategy(botAI); }
    static Strategy* tbc_mech(PlayerbotAI* botAI) { return new TbcDungeonMechanarStrategy(botAI); }
    static Strategy* tbc_ub(PlayerbotAI* botAI) { return new TbcDungeonUnderbogStrategy(botAI); }
    static Strategy* tbc_mgt(PlayerbotAI* botAI) { return new TbcDungeonMagistersTerraceStrategy(botAI); }
    static Strategy* tbc_ramp(PlayerbotAI* botAI) { return new TbcDungeonHellfireRampartsStrategy(botAI); }
    static Strategy* tbc_efd(PlayerbotAI* botAI) { return new EscapeFromDurnholdeStrategy(botAI); }
    static Strategy* tbc_bm(PlayerbotAI* botAI) { return new BlackMorassStrategy(botAI); }
    static Strategy* tbc_sv(PlayerbotAI* botAI) { return new SteamvaultStrategy(botAI); }
    static Strategy* tbc_bf(PlayerbotAI* botAI) { return new BloodFurnaceStrategy(botAI); }
    static Strategy* tbc_sp(PlayerbotAI* botAI) { return new SlavePensStrategy(botAI); }
    static Strategy* tbc_mato(PlayerbotAI* botAI) { return new ManaTombsStrategy(botAI); }
    static Strategy* tbc_shh(PlayerbotAI* botAI) { return new ShatteredHallsStrategy(botAI); }
    static Strategy* tbc_sl(PlayerbotAI* botAI) { return new ShadowLabyrinthStrategy(botAI); }
    static Strategy* tbc_arc(PlayerbotAI* botAI) { return new ArcatrazStrategy(botAI); }
    static Strategy* tbc_bot(PlayerbotAI* botAI) { return new BotanicaStrategy(botAI); }
    static Strategy* wotlk_uk(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
    static Strategy* wotlk_nex(PlayerbotAI* botAI) { return new WotlkDungeonNexStrategy(botAI); }
    static Strategy* wotlk_an(PlayerbotAI* botAI) { return new WotlkDungeonANStrategy(botAI); }
    static Strategy* wotlk_ok(PlayerbotAI* botAI) { return new WotlkDungeonOKStrategy(botAI); }
    static Strategy* wotlk_dtk(PlayerbotAI* botAI) { return new WotlkDungeonDTKStrategy(botAI); }
    static Strategy* wotlk_vh(PlayerbotAI* botAI) { return new WotlkDungeonVHStrategy(botAI); }
    static Strategy* wotlk_gd(PlayerbotAI* botAI) { return new WotlkDungeonGDStrategy(botAI); }
    static Strategy* wotlk_hos(PlayerbotAI* botAI) { return new WotlkDungeonHoSStrategy(botAI); }
    static Strategy* wotlk_hol(PlayerbotAI* botAI) { return new WotlkDungeonHoLStrategy(botAI); }
    static Strategy* wotlk_occ(PlayerbotAI* botAI) { return new WotlkDungeonOccStrategy(botAI); }
    static Strategy* wotlk_up(PlayerbotAI* botAI) { return new WotlkDungeonUPStrategy(botAI); }
    static Strategy* wotlk_cos(PlayerbotAI* botAI) { return new WotlkDungeonCoSStrategy(botAI); }
    static Strategy* wotlk_fos(PlayerbotAI* botAI) { return new WotlkDungeonFoSStrategy(botAI); }
    static Strategy* wotlk_pos(PlayerbotAI* botAI) { return new WotlkDungeonPoSStrategy(botAI); }
    static Strategy* wotlk_toc(PlayerbotAI* botAI) { return new WotlkDungeonToCStrategy(botAI); }
};

#endif
