#ifndef _PLAYERBOT_SHADOWLABYRINTHACTIONCONTEXT_H
#define _PLAYERBOT_SHADOWLABYRINTHACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "ShadowLabyrinthActions.h"

class ShadowLabyrinthActionContext : public NamedObjectContext<Action>
{
public:
    ShadowLabyrinthActionContext()
    {
        creators["avoid corrosive acid"] = &ShadowLabyrinthActionContext::avoid_corrosive_acid;
        creators["hellmaw fear react"] = &ShadowLabyrinthActionContext::hellmaw_fear_react;
        creators["incite chaos react"] = &ShadowLabyrinthActionContext::incite_chaos_react;
        creators["avoid war stomp"] = &ShadowLabyrinthActionContext::avoid_war_stomp;
        creators["blackheart charge react"] = &ShadowLabyrinthActionContext::blackheart_charge_react;
        creators["vorpil spread"] = &ShadowLabyrinthActionContext::vorpil_spread;
        creators["move from rain of fire"] = &ShadowLabyrinthActionContext::move_from_rain_of_fire;
        creators["draw shadows react"] = &ShadowLabyrinthActionContext::draw_shadows_react;
        creators["murmur sonic boom"] = &ShadowLabyrinthActionContext::murmur_sonic_boom;
        creators["murmur resonance"] = &ShadowLabyrinthActionContext::murmur_resonance;
        creators["murmur magnetic pull"] = &ShadowLabyrinthActionContext::murmur_magnetic_pull;
        creators["murmur thundering storm"] = &ShadowLabyrinthActionContext::murmur_thundering_storm;
    }

private:
    static Action* avoid_corrosive_acid(PlayerbotAI* botAI) { return new AvoidCorrosiveAcidAction(botAI); }
    static Action* hellmaw_fear_react(PlayerbotAI* botAI) { return new HellmawFearReactAction(botAI); }
    static Action* incite_chaos_react(PlayerbotAI* botAI) { return new InciteChaosReactAction(botAI); }
    static Action* avoid_war_stomp(PlayerbotAI* botAI) { return new AvoidWarStompAction(botAI); }
    static Action* blackheart_charge_react(PlayerbotAI* botAI) { return new BlackheartChargeReactAction(botAI); }
    static Action* vorpil_spread(PlayerbotAI* botAI) { return new VorpilSpreadAction(botAI); }
    static Action* move_from_rain_of_fire(PlayerbotAI* botAI) { return new MoveFromRainOfFireAction(botAI); }
    static Action* draw_shadows_react(PlayerbotAI* botAI) { return new DrawShadowsReactAction(botAI); }
    static Action* murmur_sonic_boom(PlayerbotAI* botAI) { return new MurmurSonicBoomAction(botAI); }
    static Action* murmur_resonance(PlayerbotAI* botAI) { return new MurmurResonanceAction(botAI); }
    static Action* murmur_magnetic_pull(PlayerbotAI* botAI) { return new MurmurMagneticPullAction(botAI); }
    static Action* murmur_thundering_storm(PlayerbotAI* botAI) { return new MurmurThunderingStormAction(botAI); }
};

#endif
