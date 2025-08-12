#ifndef _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_
#define _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_

#include "RaidOnyxiaStrategy.h"
#include "RaidUlduarStrategy.h"
#include "Strategy.h"
#include "RaidBwlStrategy.h"
#include "RaidNaxxStrategy.h"
#include "RaidOsStrategy.h"
#include "RaidEoEStrategy.h"
#include "RaidMcStrategy.h"
#include "RaidAq20Strategy.h"
#include "RaidIccStrategy.h"
#include "RaidVoAStrategy.h"
#include "karazhan/KarazhanStrategy.h"
#include "gruul/GruulStrategy.h"
#include "magtheridon/MagtheridonStrategy.h"
#include "serpentshrine/SerpentshrineStrategy.h"
#include "tempestkeep/TempestKeepStrategy.h"
#include "blacktemple/BlackTempleStrategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        // TODO should we give these prefixes (eg: "naxx" -> "raid naxx")? because if we don't it's going to end up
        // very crowded (with possible conflicts) once we have strats for all raids and some dungeons
        // (mc already very similiar to nc)
        creators["mc"] = &RaidStrategyContext::mc;
        creators["bwl"] = &RaidStrategyContext::bwl;
        creators["aq20"] = &RaidStrategyContext::aq20;
        creators["naxx"] = &RaidStrategyContext::naxx;
        creators["wotlk-os"] = &RaidStrategyContext::wotlk_os;
        creators["wotlk-eoe"] = &RaidStrategyContext::wotlk_eoe;
        creators["voa"] = &RaidStrategyContext::voa;
        creators["uld"] = &RaidStrategyContext::uld;
        creators["icc"] = &RaidStrategyContext::icc;
        creators["onyxia"] = &RaidStrategyContext::onyxia;
        creators["karazhan"] = &RaidStrategyContext::karazhan;
        creators["gruul"] = &RaidStrategyContext::gruul;
        creators["magtheridon"] = &RaidStrategyContext::magtheridon;
        creators["serpentshrine"] = &RaidStrategyContext::serpentshrine;
        creators["tempestkeep"] = &RaidStrategyContext::tempestkeep;
        creators["blacktemple"] = &RaidStrategyContext::blacktemple;
        creators["najentus"] = &RaidStrategyContext::najentus;
        creators["supremus"] = &RaidStrategyContext::supremus;
        creators["shade of akama"] = &RaidStrategyContext::shade_of_akama;
        creators["teron gorefiend"] = &RaidStrategyContext::teron_gorefiend;
        creators["gurtogg bloodboil"] = &RaidStrategyContext::gurtogg_bloodboil;
        creators["reliquary of souls"] = &RaidStrategyContext::reliquary_of_souls;
        creators["mother shahraz"] = &RaidStrategyContext::mother_shahraz;
    }

private:
    static Strategy* mc(PlayerbotAI* botAI) { return new RaidMcStrategy(botAI); }
    static Strategy* bwl(PlayerbotAI* botAI) { return new RaidBwlStrategy(botAI); }
    static Strategy* aq20(PlayerbotAI* botAI) { return new RaidAq20Strategy(botAI); }
    static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
    static Strategy* wotlk_os(PlayerbotAI* botAI) { return new RaidOsStrategy(botAI); }
    static Strategy* wotlk_eoe(PlayerbotAI* botAI) { return new RaidEoEStrategy(botAI); }
    static Strategy* voa(PlayerbotAI* botAI) { return new RaidVoAStrategy(botAI); }
    static Strategy* uld(PlayerbotAI* botAI) { return new RaidUlduarStrategy(botAI); }
    static Strategy* icc(PlayerbotAI* botAI) { return new RaidIccStrategy(botAI); }
    static Strategy* onyxia(PlayerbotAI* botAI) { return new RaidOnyxiaStrategy(botAI); }
    static Strategy* karazhan(PlayerbotAI* botAI) { return new KarazhanStrategy(botAI); }
    static Strategy* gruul(PlayerbotAI* botAI) { return new GruulStrategy(botAI); }
    static Strategy* magtheridon(PlayerbotAI* botAI) { return new MagtheridonStrategy(botAI); }
    static Strategy* serpentshrine(PlayerbotAI* botAI) { return new SerpentshrineStrategy(botAI); }
    static Strategy* tempestkeep(PlayerbotAI* botAI) { return new TempestKeepStrategy(botAI); }
    static Strategy* blacktemple(PlayerbotAI* botAI) { return new RaidBtStrategy(botAI); }
    static Strategy* najentus(PlayerbotAI* botAI) { return new RaidBtNajentusStrategy(botAI); }
    static Strategy* supremus(PlayerbotAI* botAI) { return new RaidBtSupremusStrategy(botAI); }
    static Strategy* shade_of_akama(PlayerbotAI* botAI) { return new RaidBtShadeOfAkamaStrategy(botAI); }
    static Strategy* teron_gorefiend(PlayerbotAI* botAI) { return new RaidBtTeronGorefiendStrategy(botAI); }
    static Strategy* gurtogg_bloodboil(PlayerbotAI* botAI) { return new RaidBtGurtoggBloodboilStrategy(botAI); }
    static Strategy* reliquary_of_souls(PlayerbotAI* botAI) { return new RaidBtReliquaryOfSoulsStrategy(botAI); }
    static Strategy* mother_shahraz(PlayerbotAI* botAI) { return new RaidBtMotherShahrazStrategy(botAI); }
};

#endif
