#ifndef _PLAYERBOT_MAGTHERIDONTRIGGERCONTEXT_H
#define _PLAYERBOT_MAGTHERIDONTRIGGERCONTEXT_H

#include "Trigger.h"
#include "NamedObjectContext.h"
#include "MagtheridonTriggers.h"

class MagtheridonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MagtheridonTriggerContext()
    {
        creators["hellfire channeler near"] = &MagtheridonTriggerContext::hellfire_channeler_near;
        creators["dark mending cast"] = &MagtheridonTriggerContext::dark_mending_cast;
        creators["blast nova cast"] = &MagtheridonTriggerContext::blast_nova_cast;
        creators["stop cube channel"] = &MagtheridonTriggerContext::stop_cube_channel;
        creators["quake cast"] = &MagtheridonTriggerContext::quake_cast;
        creators["debris falling"] = &MagtheridonTriggerContext::debris_falling;
        creators["cave in"] = &MagtheridonTriggerContext::cave_in;
        creators["blaze nearby"] = &MagtheridonTriggerContext::blaze_nearby;
        creators["cleave danger"] = &MagtheridonTriggerContext::cleave_danger;
        creators["hellfire warder active"] = &MagtheridonTriggerContext::hellfire_warder_active;
        creators["tank position adds"] = &MagtheridonTriggerContext::tank_position_adds;
        creators["healer position needed"] = &MagtheridonTriggerContext::healer_position_needed;
        creators["banish phase active"] = &MagtheridonTriggerContext::banish_phase_active;
        creators["infernal nearby"] = &MagtheridonTriggerContext::infernal_nearby;
        creators["channeler interrupt needed"] = &MagtheridonTriggerContext::channeler_interrupt_needed;
        creators["magtheridon phase transition"] = &MagtheridonTriggerContext::magtheridon_phase_transition;
        creators["shadow bolt volley cast"] = &MagtheridonTriggerContext::shadow_bolt_volley_cast;
        creators["mind exhaustion check"] = &MagtheridonTriggerContext::mind_exhaustion_check;
        creators["magtheridon released"] = &MagtheridonTriggerContext::magtheridon_released;
        creators["magtheridon low health"] = &MagtheridonTriggerContext::magtheridon_low_health;
    }

private:
    static Trigger* hellfire_channeler_near(PlayerbotAI* ai) { return new HellfireChannelerNearTrigger(ai); }
    static Trigger* dark_mending_cast(PlayerbotAI* ai) { return new DarkMendingCastTrigger(ai); }
    static Trigger* blast_nova_cast(PlayerbotAI* ai) { return new BlastNovaCastTrigger(ai); }
    static Trigger* stop_cube_channel(PlayerbotAI* ai) { return new StopCubeChannelTrigger(ai); }
    static Trigger* quake_cast(PlayerbotAI* ai) { return new QuakeCastTrigger(ai); }
    static Trigger* debris_falling(PlayerbotAI* ai) { return new DebrisFallingTrigger(ai); }
    static Trigger* cave_in(PlayerbotAI* ai) { return new CaveInTrigger(ai); }
    static Trigger* blaze_nearby(PlayerbotAI* ai) { return new BlazeNearbyTrigger(ai); }
    static Trigger* cleave_danger(PlayerbotAI* ai) { return new CleaveDangerTrigger(ai); }
    static Trigger* hellfire_warder_active(PlayerbotAI* ai) { return new HellfireWarderActiveTrigger(ai); }
    static Trigger* tank_position_adds(PlayerbotAI* ai) { return new TankPositionAddsTrigger(ai); }
    static Trigger* healer_position_needed(PlayerbotAI* ai) { return new HealerPositionNeededTrigger(ai); }
    static Trigger* banish_phase_active(PlayerbotAI* ai) { return new BanishPhaseActiveTrigger(ai); }
    static Trigger* infernal_nearby(PlayerbotAI* ai) { return new InfernalNearbyTrigger(ai); }
    static Trigger* channeler_interrupt_needed(PlayerbotAI* ai) { return new ChannelerInterruptNeededTrigger(ai); }
    static Trigger* magtheridon_phase_transition(PlayerbotAI* ai) { return new MagtheridonPhaseTransitionTrigger(ai); }
    static Trigger* shadow_bolt_volley_cast(PlayerbotAI* ai) { return new ShadowBoltVolleyCastTrigger(ai); }
    static Trigger* mind_exhaustion_check(PlayerbotAI* ai) { return new MindExhaustionCheckTrigger(ai); }
    static Trigger* magtheridon_released(PlayerbotAI* ai) { return new MagtheridonReleasedTrigger(ai); }
    static Trigger* magtheridon_low_health(PlayerbotAI* ai) { return new MagtheridonLowHealthTrigger(ai); }
};

#endif