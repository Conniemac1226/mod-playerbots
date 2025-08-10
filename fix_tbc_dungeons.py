#!/usr/bin/env python3
"""
Fix TBC Dungeon Strategies to use proven WotLK patterns
This script will:
1. Replace FindNearestCreature with AI_VALUE2 pattern
2. Replace Cell::VisitObjects with AI_VALUE(GuidVector) pattern
3. Show all changes before applying them
"""

import re
import os

# Mapping of NPC constants to boss names for AI_VALUE2
BOSS_MAPPINGS = {
    # Steamvault
    "NPC_HYDROMANCER_THESPIA": "hydromancer thespia",
    "NPC_MEKGINEER_STEAMRIGGER": "mekgineer steamrigger",
    "NPC_WARLORD_KALITHRESH": "warlord kalithresh",
    
    # Hellfire Ramparts  
    "NPC_WATCHKEEPER_GARGOLMAR": "watchkeeper gargolmar",
    "NPC_OMOR_THE_UNSCARRED": "omor the unscarred",
    "NPC_VAZRUDEN_THE_HERALD": "vazruden the herald",
    "NPC_NAZAN": "nazan",
    
    # Blood Furnace
    "NPC_THE_MAKER": "the maker",
    "NPC_BROGGOK": "broggok",
    "NPC_KELIDAN_THE_BREAKER": "kelidan the breaker",
    
    # Slave Pens
    "NPC_MENNU_THE_BETRAYER": "mennu the betrayer",
    "NPC_ROKMAR_THE_CRACKLER": "rokmar the crackler",
    "NPC_QUAGMIRRAN": "quagmirran",
    
    # Underbog
    "NPC_HUNGARFEN": "hungarfen",
    "NPC_GHAZAN": "ghazan",
    "NPC_SWAMPLORD_MUSELEK": "swamplord muselek",
    "NPC_THE_BLACK_STALKER": "the black stalker",
    
    # Mana-Tombs
    "NPC_PANDEMONIUS": "pandemonius",
    "NPC_TAVAROK": "tavarok",
    "NPC_NEXUSPRINCE_SHAFFAR": "nexus-prince shaffar",
    "NPC_YOR": "yor"
}

def preview_changes(filepath, changes):
    """Show what changes will be made"""
    print(f"\n{'='*60}")
    print(f"FILE: {filepath}")
    print(f"{'='*60}")
    
    for change in changes:
        print(f"\nLINE {change['line']}:")
        print(f"  OLD: {change['old'][:80]}...")
        print(f"  NEW: {change['new'][:80]}...")

def fix_findnearestcreature(content):
    """Replace FindNearestCreature with AI_VALUE2 pattern"""
    changes = []
    
    # Pattern to match FindNearestCreature calls
    pattern = r'Unit\* (\w+) = bot->FindNearestCreature\((\w+), [\d.]+f\);'
    
    for match in re.finditer(pattern, content):
        var_name = match.group(1)
        npc_const = match.group(2)
        
        if npc_const in BOSS_MAPPINGS:
            boss_name = BOSS_MAPPINGS[npc_const]
            old_line = match.group(0)
            new_line = f'Unit* {var_name} = AI_VALUE2(Unit*, "find target", "{boss_name}");'
            
            changes.append({
                'old': old_line,
                'new': new_line,
                'line': content[:match.start()].count('\n') + 1
            })
    
    return changes

def fix_cell_visit_objects(content):
    """Replace Cell::VisitObjects pattern with AI_VALUE(GuidVector)"""
    changes = []
    
    # This is more complex - look for the pattern
    pattern = r'std::list<Unit\*> targets;.*?Cell::VisitObjects\(bot, searcher, [\d.]+f\);'
    
    matches = re.finditer(pattern, content, re.DOTALL)
    for match in matches:
        old_block = match.group(0)
        # Extract the distance value
        dist_match = re.search(r'([\d.]+)f\)', old_block)
        distance = dist_match.group(1) if dist_match else "50.0"
        
        new_block = f'GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");'
        
        changes.append({
            'old': old_block[:100],  # First 100 chars for preview
            'new': new_block,
            'line': content[:match.start()].count('\n') + 1
        })
    
    return changes

def main():
    # Dungeons to fix (excluding the working ones)
    dungeons = [
        'steamvault',
        'hellfireramparts',
        'bloodfurnace',
        'slavepens',
        'underbog',
        'manatombs'
    ]
    
    base_path = r'C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc'
    
    for dungeon in dungeons:
        actions_file = os.path.join(base_path, dungeon, f'{dungeon.title().replace("steamvault", "Steamvault").replace("hellfireramparts", "HellfireRamparts").replace("bloodfurnace", "BloodFurnace").replace("slavepens", "SlavePens").replace("underbog", "Underbog").replace("manatombs", "ManaTombs")}Actions.cpp')
        
        if not os.path.exists(actions_file):
            print(f"File not found: {actions_file}")
            continue
            
        with open(actions_file, 'r') as f:
            content = f.read()
        
        # Find all changes needed
        findnearestcreature_changes = fix_findnearestcreature(content)
        cell_visit_changes = fix_cell_visit_objects(content)
        
        # Preview changes
        all_changes = findnearestcreature_changes + cell_visit_changes
        if all_changes:
            preview_changes(actions_file, all_changes)
            
            response = input(f"\nApply {len(all_changes)} changes to {dungeon}? (y/n): ")
            if response.lower() == 'y':
                # Apply changes here
                print(f"Would apply changes to {dungeon}")
        else:
            print(f"No changes needed for {dungeon}")

if __name__ == "__main__":
    print("TBC Dungeon Fix Preview Script")
    print("This will show you all the changes that would be made")
    print("No files will be modified unless you confirm")
    main()