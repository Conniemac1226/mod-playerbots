#ifndef _PLAYERBOT_GRUULACTIONCONTEXT_H
#define _PLAYERBOT_GRUULACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "GruulActions.h"

class GruulActionContext : public NamedObjectContext<Action>
{
public:
    GruulActionContext()
    {
        // Gruul actions
        creators["gruul ground slam"] = &GruulActionContext::gruul_ground_slam;
        creators["gruul shatter position"] = &GruulActionContext::gruul_shatter_position;
        creators["gruul cave in"] = &GruulActionContext::gruul_cave_in;
        creators["gruul hurtful strike"] = &GruulActionContext::gruul_hurtful_strike;
        creators["gruul tank swap"] = &GruulActionContext::gruul_tank_swap;
        creators["gruul dispel"] = &GruulActionContext::gruul_dispel;
        
        // Maulgar actions
        creators["maulgar focus target"] = &GruulActionContext::maulgar_focus_target;
        creators["maulgar position"] = &GruulActionContext::maulgar_position;
        creators["maulgar whirlwind"] = &GruulActionContext::maulgar_whirlwind;
        creators["maulgar arcing smash"] = &GruulActionContext::maulgar_arcing_smash;
        
        // Council actions
        creators["krosh spellsteal"] = &GruulActionContext::krosh_spellsteal;
        creators["kiggler polymorph"] = &GruulActionContext::kiggler_polymorph;
        creators["olm wild fel stalker"] = &GruulActionContext::olm_wild_fel_stalker;
        creators["blindeye interrupt"] = &GruulActionContext::blindeye_interrupt;
        creators["krosh blast wave avoid"] = &GruulActionContext::krosh_blast_wave_avoid;
        creators["kiggler arcane explosion avoid"] = &GruulActionContext::kiggler_arcane_explosion_avoid;
    }
    
private:
    // Gruul actions
    static Action* gruul_ground_slam(PlayerbotAI* ai) { return new GruulGroundSlamAction(ai); }
    static Action* gruul_shatter_position(PlayerbotAI* ai) { return new GruulShatterPositionAction(ai); }
    static Action* gruul_cave_in(PlayerbotAI* ai) { return new GruulCaveInAction(ai); }
    static Action* gruul_hurtful_strike(PlayerbotAI* ai) { return new GruulHurtfulStrikeAction(ai); }
    static Action* gruul_tank_swap(PlayerbotAI* ai) { return new GruulTankSwapAction(ai); }
    static Action* gruul_dispel(PlayerbotAI* ai) { return new GruulDispelAction(ai); }
    
    // Maulgar actions
    static Action* maulgar_focus_target(PlayerbotAI* ai) { return new MaulgarFocusTargetAction(ai); }
    static Action* maulgar_position(PlayerbotAI* ai) { return new MaulgarPositionAction(ai); }
    static Action* maulgar_whirlwind(PlayerbotAI* ai) { return new MaulgarWhirlwindAction(ai); }
    static Action* maulgar_arcing_smash(PlayerbotAI* ai) { return new MaulgarArcingSmashAction(ai); }
    
    // Council actions
    static Action* krosh_spellsteal(PlayerbotAI* ai) { return new KroshSpellstealAction(ai); }
    static Action* kiggler_polymorph(PlayerbotAI* ai) { return new KigglerPolymorphAction(ai); }
    static Action* olm_wild_fel_stalker(PlayerbotAI* ai) { return new OlmWildFelStalkerAction(ai); }
    static Action* blindeye_interrupt(PlayerbotAI* ai) { return new BlindeyeInterruptAction(ai); }
    static Action* krosh_blast_wave_avoid(PlayerbotAI* ai) { return new KroshBlastWaveAvoidAction(ai); }
    static Action* kiggler_arcane_explosion_avoid(PlayerbotAI* ai) { return new KigglerArcaneExplosionAvoidAction(ai); }
};

#endif
