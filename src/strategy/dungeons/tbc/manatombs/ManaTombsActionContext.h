#ifndef _PLAYERBOT_MANATOMBSACTIONCONTEXT_H
#define _PLAYERBOT_MANATOMBSACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "ManaTombsActions.h"

class ManaTombsActionContext : public NamedObjectContext<Action>
{
public:
    ManaTombsActionContext()
    {
        creators["avoid dark shell"] = &ManaTombsActionContext::avoid_dark_shell;
        creators["spread void blast"] = &ManaTombsActionContext::spread_void_blast;
        creators["avoid earthquake"] = &ManaTombsActionContext::avoid_earthquake;
        creators["break crystal prison"] = &ManaTombsActionContext::break_crystal_prison;
        creators["avoid arcing smash"] = &ManaTombsActionContext::avoid_arcing_smash;
        creators["attack ethereal beacon"] = &ManaTombsActionContext::attack_ethereal_beacon;
        creators["avoid frost nova"] = &ManaTombsActionContext::avoid_frost_nova;
        creators["shaffar blink repositioning"] = &ManaTombsActionContext::shaffar_blink_repositioning;
        creators["avoid double breath"] = &ManaTombsActionContext::avoid_double_breath;
        creators["avoid stomp"] = &ManaTombsActionContext::avoid_stomp;
    }

private:
    static Action* avoid_dark_shell(PlayerbotAI* ai) { return new PandemoniusDarkShellAction(ai); }
    static Action* spread_void_blast(PlayerbotAI* ai) { return new PandemoniusVoidBlastAction(ai); }
    static Action* avoid_earthquake(PlayerbotAI* ai) { return new TavarokEarthquakeAction(ai); }
    static Action* break_crystal_prison(PlayerbotAI* ai) { return new TavarokCrystalPrisonAction(ai); }
    static Action* avoid_arcing_smash(PlayerbotAI* ai) { return new TavarokArcingSmashAction(ai); }
    static Action* attack_ethereal_beacon(PlayerbotAI* ai) { return new AttackEtherealBeaconAction(ai); }
    static Action* avoid_frost_nova(PlayerbotAI* ai) { return new ShaffarFrostNovaAction(ai); }
    static Action* shaffar_blink_repositioning(PlayerbotAI* ai) { return new ShaffarBlinkAction(ai); }
    static Action* avoid_double_breath(PlayerbotAI* ai) { return new YorDoubleBreathAction(ai); }
    static Action* avoid_stomp(PlayerbotAI* ai) { return new YorStompAction(ai); }
};

#endif