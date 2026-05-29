#ifndef _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_
#define _PLAYERBOT_RAIDSTRATEGYCONTEXT_H_

#include "Strategy.h"
#include "RaidAq20Strategy.h"
#include "RaidMcStrategy.h"
#include "RaidBwlStrategy.h"
#include "RaidKarazhanStrategy.h"
#include "RaidGruulsLairStrategy.h"
#include "RaidMagtheridonStrategy.h"
#include "RaidNaxxStrategy.h"
#include "RaidSSCStrategy.h"
#include "RaidTempestKeepStrategy.h"
#include "HyjalSummit/Strategy/RaidHyjalSummitStrategy.h"
#include "RaidZulAmanStrategy.h"
#include "RaidOsStrategy.h"
#include "RaidEoEStrategy.h"
#include "RaidVoAStrategy.h"
#include "RaidUlduarStrategy.h"
#include "RaidOnyxiaStrategy.h"
#include "ICCStrategy.h"
#include "../../strategy/raids/blacktemple/BlackTempleStrategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        // Official WotLK/Refactored
        creators["aq20"] = &RaidStrategyContext::aq20;
        creators["moltencore"] = &RaidStrategyContext::moltencore;
        creators["bwl"] = &RaidStrategyContext::bwl;
        creators["karazhan"] = &RaidStrategyContext::karazhan;
        creators["gruulslair"] = &RaidStrategyContext::gruulslair;
        creators["magtheridon"] = &RaidStrategyContext::magtheridon;
        creators["naxx"] = &RaidStrategyContext::naxx;
        creators["ssc"] = &RaidStrategyContext::ssc;
        creators["tempestkeep"] = &RaidStrategyContext::tempestkeep;
        creators["hyjal"] = &RaidStrategyContext::hyjal;
        creators["zulaman"] = &RaidStrategyContext::zulaman;
        creators["wotlk-os"] = &RaidStrategyContext::wotlk_os;
        creators["wotlk-eoe"] = &RaidStrategyContext::wotlk_eoe;
        creators["voa"] = &RaidStrategyContext::voa;
        creators["ulduar"] = &RaidStrategyContext::ulduar;
        creators["onyxia"] = &RaidStrategyContext::onyxia;
        creators["icc"] = &RaidStrategyContext::icc;

        // Custom / Unique to this branch
        creators["blacktemple"] = &RaidStrategyContext::blacktemple;
        creators["najentus"] = &RaidStrategyContext::najentus;
        creators["supremus"] = &RaidStrategyContext::supremus;
        creators["shade of akama"] = &RaidStrategyContext::shade_of_akama;
        creators["teron gorefiend"] = &RaidStrategyContext::teron_gorefiend;
        creators["gurtogg bloodboil"] = &RaidStrategyContext::gurtogg_bloodboil;
        creators["reliquary of souls"] = &RaidStrategyContext::reliquary_of_souls;
        creators["mother shahraz"] = &RaidStrategyContext::mother_shahraz;
        creators["illidari council"] = &RaidStrategyContext::illidari_council;
        creators["illidan stormrage"] = &RaidStrategyContext::illidan_stormrage;
    }

private:
    static Strategy* aq20(PlayerbotAI* botAI) { return new RaidAq20Strategy(botAI); }
    static Strategy* moltencore(PlayerbotAI* botAI) { return new RaidMcStrategy(botAI); }
    static Strategy* bwl(PlayerbotAI* botAI) { return new RaidBwlStrategy(botAI); }
    static Strategy* karazhan(PlayerbotAI* botAI) { return new RaidKarazhanStrategy(botAI); }
    static Strategy* gruulslair(PlayerbotAI* botAI) { return new RaidGruulsLairStrategy(botAI); }
    static Strategy* magtheridon(PlayerbotAI* botAI) { return new RaidMagtheridonStrategy(botAI); }
    static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
    static Strategy* ssc(PlayerbotAI* botAI) { return new RaidSSCStrategy(botAI); }
    static Strategy* tempestkeep(PlayerbotAI* botAI) { return new RaidTempestKeepStrategy(botAI); }
    static Strategy* hyjal(PlayerbotAI* botAI) { return new RaidHyjalSummitStrategy(botAI); }
    static Strategy* zulaman(PlayerbotAI* botAI) { return new RaidZulAmanStrategy(botAI); }
    static Strategy* wotlk_os(PlayerbotAI* botAI) { return new RaidOsStrategy(botAI); }
    static Strategy* wotlk_eoe(PlayerbotAI* botAI) { return new RaidEoEStrategy(botAI); }
    static Strategy* voa(PlayerbotAI* botAI) { return new RaidVoAStrategy(botAI); }
    static Strategy* onyxia(PlayerbotAI* botAI) { return new RaidOnyxiaStrategy(botAI); }
    static Strategy* ulduar(PlayerbotAI* botAI) { return new RaidUlduarStrategy(botAI); }
    static Strategy* icc(PlayerbotAI* botAI) { return new RaidIccStrategy(botAI); }

    // Custom
    static Strategy* blacktemple(PlayerbotAI* botAI) { return new RaidBtStrategy(botAI); }
    static Strategy* najentus(PlayerbotAI* botAI) { return new RaidBtNajentusStrategy(botAI); }
    static Strategy* supremus(PlayerbotAI* botAI) { return new RaidBtSupremusStrategy(botAI); }
    static Strategy* shade_of_akama(PlayerbotAI* botAI) { return new RaidBtShadeOfAkamaStrategy(botAI); }
    static Strategy* teron_gorefiend(PlayerbotAI* botAI) { return new RaidBtTeronGorefiendStrategy(botAI); }
    static Strategy* gurtogg_bloodboil(PlayerbotAI* botAI) { return new RaidBtGurtoggBloodboilStrategy(botAI); }
    static Strategy* reliquary_of_souls(PlayerbotAI* botAI) { return new RaidBtReliquaryOfSoulsStrategy(botAI); }
    static Strategy* mother_shahraz(PlayerbotAI* botAI) { return new RaidBtMotherShahrazStrategy(botAI); }
    static Strategy* illidari_council(PlayerbotAI* botAI) { return new RaidBtIllidariCouncilStrategy(botAI); }
    static Strategy* illidan_stormrage(PlayerbotAI* botAI) { return new RaidBtIllidanStormrageStrategy(botAI); }
};

#endif
