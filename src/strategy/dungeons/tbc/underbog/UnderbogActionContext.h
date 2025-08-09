#ifndef _PLAYERBOT_UNDERBOGACTIONCONTEXT_H
#define _PLAYERBOT_UNDERBOGACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "UnderbogActions.h"

class UnderbogActionContext : public NamedObjectContext<Action>
{
public:
    UnderbogActionContext()
    {
        creators["avoid mushroom explosion"] = &UnderbogActionContext::avoid_mushroom_explosion;
        creators["avoid foul spores"] = &UnderbogActionContext::avoid_foul_spores;
        creators["avoid acid breath"] = &UnderbogActionContext::avoid_acid_breath;
        creators["avoid tail sweep"] = &UnderbogActionContext::avoid_tail_sweep;
        creators["attack windcaller claw"] = &UnderbogActionContext::attack_windcaller_claw;
        creators["avoid freezing trap"] = &UnderbogActionContext::avoid_freezing_trap;
        creators["dispel hunters mark"] = &UnderbogActionContext::dispel_hunters_mark;
        creators["position for levitate"] = &UnderbogActionContext::position_for_levitate;
        creators["attack spore strider"] = &UnderbogActionContext::attack_spore_strider;
        creators["interrupt chain lightning"] = &UnderbogActionContext::interrupt_chain_lightning;
    }

private:
    static Action* avoid_mushroom_explosion(PlayerbotAI* ai) { return new HungarfenMushroomAction(ai); }
    static Action* avoid_foul_spores(PlayerbotAI* ai) { return new HungarfenFoulSporesAction(ai); }
    static Action* avoid_acid_breath(PlayerbotAI* ai) { return new GhazanAcidBreathAction(ai); }
    static Action* avoid_tail_sweep(PlayerbotAI* ai) { return new GhazanTailSweepAction(ai); }
    static Action* attack_windcaller_claw(PlayerbotAI* ai) { return new AttackWindcallerClawAction(ai); }
    static Action* avoid_freezing_trap(PlayerbotAI* ai) { return new MuselekFreezingTrapAction(ai); }
    static Action* dispel_hunters_mark(PlayerbotAI* ai) { return new MuselekHuntersMarkAction(ai); }
    static Action* position_for_levitate(PlayerbotAI* ai) { return new BlackStalkerLevitateAction(ai); }
    static Action* attack_spore_strider(PlayerbotAI* ai) { return new AttackSporeStriderAction(ai); }
    static Action* interrupt_chain_lightning(PlayerbotAI* ai) { return new BlackStalkerChainLightningAction(ai); }
};

#endif