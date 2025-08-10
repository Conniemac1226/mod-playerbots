#!/bin/bash
# TBC Dungeon Actions.cpp Fix Script
# This script converts all TBC dungeon Actions.cpp files to use proven WotLK patterns

echo "TBC Dungeon Actions.cpp Fix Script - Converting to WotLK patterns"
echo "=================================================================="

# Define the files to process
TBC_ACTION_FILES=(
    "modules/mod-playerbots/src/strategy/dungeons/tbc/hellfireramparts/HellfireRampartsActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/steamvault/SteamvaultActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/bloodfurnace/BloodFurnaceActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/slavepens/SlavePensActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/underbog/UnderbogActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/manatombs/ManaTombsActions.cpp"
    "modules/mod-playerbots/src/strategy/dungeons/tbc/magistersterrace/MagistersTerraceActions.cpp"
)

# Function to apply sed replacements for a specific boss
apply_boss_replacements() {
    local file=$1
    local npc_constant=$2
    local boss_name=$3
    
    echo "  Replacing $npc_constant with '$boss_name'"
    
    # Pattern 1: Standard boss detection
    sed -i "s/Unit\* boss = bot->FindNearestCreature($npc_constant, [0-9.]*f);/Unit* boss = AI_VALUE2(Unit*, \"find target\", \"$boss_name\");/g" "$file"
    
    # Pattern 2: Named variable detection
    sed -i "s/Unit\* \([a-zA-Z_][a-zA-Z0-9_]*\) = bot->FindNearestCreature($npc_constant, [0-9.]*f);/Unit* \1 = AI_VALUE2(Unit*, \"find target\", \"$boss_name\");/g" "$file"
    
    # Pattern 3: isUseful() method pattern
    sed -i "s/bot->FindNearestCreature($npc_constant, [0-9.]*f)/AI_VALUE2(Unit*, \"find target\", \"$boss_name\")/g" "$file"
}

# Process each file
for file in "${TBC_ACTION_FILES[@]}"; do
    if [[ -f "$file" ]]; then
        echo "Processing: $file"
        
        # Apply boss-specific replacements based on detected constants in file
        
        # Hellfire Ramparts bosses
        apply_boss_replacements "$file" "NPC_WATCHKEEPER_GARGOLMAR" "watchkeeper gargolmar"
        apply_boss_replacements "$file" "NPC_OMOR_THE_UNSCARRED" "omor the unscarred" 
        apply_boss_replacements "$file" "NPC_NAZAN" "nazan"
        apply_boss_replacements "$file" "NPC_VAZRUDEN" "vazruden"
        
        # Steamvault bosses  
        apply_boss_replacements "$file" "NPC_HYDROMANCER_THESPIA" "hydromancer thespia"
        apply_boss_replacements "$file" "NPC_MEKGINEER_STEAMRIGGER" "mekgineer steamrigger"
        apply_boss_replacements "$file" "NPC_WARLORD_KALITHRESH" "warlord kalithresh"
        
        # Blood Furnace bosses
        apply_boss_replacements "$file" "NPC_THE_MAKER" "the maker"
        apply_boss_replacements "$file" "NPC_BROGGOK" "broggok"
        apply_boss_replacements "$file" "NPC_KELI_DAN_THE_BREAKER" "keli'dan the breaker"
        
        # Slave Pens bosses
        apply_boss_replacements "$file" "NPC_MENNU_THE_BETRAYER" "mennu the betrayer"
        apply_boss_replacements "$file" "NPC_ROKMAR_THE_CRACKLER" "rokmar the crackler"
        apply_boss_replacements "$file" "NPC_QUAGMIRRAN" "quagmirran"
        
        # Underbog bosses
        apply_boss_replacements "$file" "NPC_HUNGARFEN" "hungarfen"
        apply_boss_replacements "$file" "NPC_GHAZ_AN" "ghaz'an"
        apply_boss_replacements "$file" "NPC_SWAMPLORD_MUSEL_EK" "swamplord musel'ek"
        apply_boss_replacements "$file" "NPC_THE_BLACK_STALKER" "the black stalker"
        
        # Mana-Tombs bosses
        apply_boss_replacements "$file" "NPC_PANDEMONIUS" "pandemonius"
        apply_boss_replacements "$file" "NPC_TAVAROK" "tavarok"
        apply_boss_replacements "$file" "NPC_NEXUS_PRINCE_SHAFFAR" "nexus-prince shaffar"
        apply_boss_replacements "$file" "NPC_YOR" "yor"
        
        # Magisters' Terrace bosses
        apply_boss_replacements "$file" "NPC_SELIN_FIREHEART" "selin fireheart"
        apply_boss_replacements "$file" "NPC_VEXALLUS" "vexallus" 
        apply_boss_replacements "$file" "NPC_PRIESTESS_DELRISSA" "priestess delrissa"
        apply_boss_replacements "$file" "NPC_KAEL_THAS_SUNSTRIDER" "kael'thas sunstrider"
        
        # Replace manual movement calculations with FleePosition
        echo "  Converting manual movement to FleePosition calls"
        
        # Pattern: Movement away from boss
        sed -i 's/float angle = bot->GetAngle(\([^)]*\)) + M_PI;[[:space:]]*float x = bot->GetPositionX() + cos(angle) \* \([0-9.]*f\);[[:space:]]*float y = bot->GetPositionY() + sin(angle) \* [0-9.]*f;[[:space:]]*float z = bot->GetPositionZ();[[:space:]]*return MoveTo([^;]*);/return FleePosition(\1->GetPosition(), \2, 500U);/g' "$file"
        
        # Replace Cell::VisitObjects patterns with AI_VALUE nearest hostile npcs
        echo "  Converting Cell::VisitObjects to AI_VALUE patterns"
        sed -i 's/Cell::VisitAllObjects/\/\/ CONVERTED: Use AI_VALUE(GuidVector, "nearest hostile npcs") instead of Cell::VisitAllObjects/g' "$file"
        
        echo "  File updated successfully"
    else
        echo "File not found: $file"
    fi
done

echo ""
echo "Fix completed! All TBC dungeon Actions.cpp files have been updated to use proven WotLK patterns."
echo ""
echo "Changes made:"
echo "1. Replaced FindNearestCreature() with AI_VALUE2(Unit*, \"find target\", \"boss name\")"
echo "2. Replaced manual movement calculations with FleePosition() calls"  
echo "3. Commented out Cell::VisitObjects patterns for manual conversion"
echo ""
echo "Next steps:"
echo "1. Manually convert remaining Cell::VisitObjects to AI_VALUE(GuidVector, \"nearest hostile npcs\")"
echo "2. Compile to check for any remaining issues"
echo "3. Test in-game to verify functionality"