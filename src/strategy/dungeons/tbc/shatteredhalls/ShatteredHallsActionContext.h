#ifndef _PLAYERBOT_SHATTEREDHALLSACTIONCONTEXT_H
#define _PLAYERBOT_SHATTEREDHALLSACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "ShatteredHallsActions.h"

class ShatteredHallsActionContext : public NamedObjectContext<Action>
{
public:
    ShatteredHallsActionContext()
    {
        creators["avoid shadow fissure"] = &ShatteredHallsActionContext::avoid_shadow_fissure;
        creators["avoid dark spin"] = &ShatteredHallsActionContext::avoid_dark_spin;
        creators["move from blast wave"] = &ShatteredHallsActionContext::move_from_blast_wave;
        creators["avoid burning maul"] = &ShatteredHallsActionContext::avoid_burning_maul;
        creators["avoid blade dance"] = &ShatteredHallsActionContext::avoid_blade_dance;
        creators["kill shattered assassins"] = &ShatteredHallsActionContext::kill_shattered_assassins;
        creators["nethekurse peon priority"] = &ShatteredHallsActionContext::nethekurse_peon_priority;
        creators["avoid flame arrow fire"] = &ShatteredHallsActionContext::avoid_flame_arrow_fire;
    }

private:
    static Action* avoid_shadow_fissure(PlayerbotAI* botAI) { return new AvoidShadowFissureAction(botAI); }
    static Action* avoid_dark_spin(PlayerbotAI* botAI) { return new AvoidDarkSpinAction(botAI); }
    static Action* move_from_blast_wave(PlayerbotAI* botAI) { return new MoveFromBlastWaveAction(botAI); }
    static Action* avoid_burning_maul(PlayerbotAI* botAI) { return new AvoidBurningMaulAction(botAI); }
    static Action* avoid_blade_dance(PlayerbotAI* botAI) { return new AvoidBladeDanceAction(botAI); }
    static Action* kill_shattered_assassins(PlayerbotAI* botAI) { return new KillShatteredAssassinsAction(botAI); }
    static Action* nethekurse_peon_priority(PlayerbotAI* botAI) { return new NetheKursePeonPriorityAction(botAI); }
    static Action* avoid_flame_arrow_fire(PlayerbotAI* botAI) { return new AvoidFlameArrowFireAction(botAI); }
};

#endif