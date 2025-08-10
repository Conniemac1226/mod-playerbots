#ifndef _PLAYERBOT_MAGTHERIDONACTIONCONTEXT_H
#define _PLAYERBOT_MAGTHERIDONACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "MagtheridonActions.h"

class MagtheridonActionContext : public NamedObjectContext<Action>
{
public:
    MagtheridonActionContext()
    {
        creators["hellfire channeler target"] = &MagtheridonActionContext::hellfire_channeler_target;
        creators["interrupt dark mending"] = &MagtheridonActionContext::interrupt_dark_mending;
        creators["click manticron cube"] = &MagtheridonActionContext::click_manticron_cube;
        creators["stop clicking cube"] = &MagtheridonActionContext::stop_clicking_cube;
        creators["avoid quake"] = &MagtheridonActionContext::avoid_quake;
        creators["avoid debris"] = &MagtheridonActionContext::avoid_debris;
        creators["handle cave in"] = &MagtheridonActionContext::handle_cave_in;
        creators["avoid blaze"] = &MagtheridonActionContext::avoid_blaze;
        creators["spread for cleave"] = &MagtheridonActionContext::spread_for_cleave;
        creators["hellfire warder target"] = &MagtheridonActionContext::hellfire_warder_target;
        creators["tank position adds"] = &MagtheridonActionContext::tank_position_adds;
        creators["healer position magtheridon"] = &MagtheridonActionContext::healer_position_magtheridon;
        creators["banish phase position"] = &MagtheridonActionContext::banish_phase_position;
        creators["avoid infernal"] = &MagtheridonActionContext::avoid_infernal;
        creators["coordinate channeler interrupt"] = &MagtheridonActionContext::coordinate_channeler_interrupt;
        creators["magtheridon phase transition"] = &MagtheridonActionContext::magtheridon_phase_transition;
    }

private:
    static Action* hellfire_channeler_target(PlayerbotAI* ai) { return new HellfireChannelerTargetAction(ai); }
    static Action* interrupt_dark_mending(PlayerbotAI* ai) { return new InterruptDarkMendingAction(ai); }
    static Action* click_manticron_cube(PlayerbotAI* ai) { return new ClickManticronCubeAction(ai); }
    static Action* stop_clicking_cube(PlayerbotAI* ai) { return new StopClickingCubeAction(ai); }
    static Action* avoid_quake(PlayerbotAI* ai) { return new AvoidQuakeAction(ai); }
    static Action* avoid_debris(PlayerbotAI* ai) { return new AvoidDebrisAction(ai); }
    static Action* handle_cave_in(PlayerbotAI* ai) { return new HandleCaveInAction(ai); }
    static Action* avoid_blaze(PlayerbotAI* ai) { return new AvoidBlazeAction(ai); }
    static Action* spread_for_cleave(PlayerbotAI* ai) { return new SpreadForCleaveAction(ai); }
    static Action* hellfire_warder_target(PlayerbotAI* ai) { return new HellfireWarderTargetAction(ai); }
    static Action* tank_position_adds(PlayerbotAI* ai) { return new TankPositionAddsAction(ai); }
    static Action* healer_position_magtheridon(PlayerbotAI* ai) { return new HealerPositionMagtheridonAction(ai); }
    static Action* banish_phase_position(PlayerbotAI* ai) { return new BanishPhasePositionAction(ai); }
    static Action* avoid_infernal(PlayerbotAI* ai) { return new AvoidInfernalAction(ai); }
    static Action* coordinate_channeler_interrupt(PlayerbotAI* ai) { return new CoordinateChannelerInterruptAction(ai); }
    static Action* magtheridon_phase_transition(PlayerbotAI* ai) { return new MagtheridonPhaseTransitionAction(ai); }
};

#endif