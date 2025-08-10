# TBC Dungeon Actions.cpp Fix Script
# This script converts all TBC dungeon Actions.cpp files to use proven WotLK patterns

# Boss name mappings from NPC constants to AI_VALUE2 names
$BossNameMap = @{
    # Hellfire Ramparts
    "NPC_WATCHKEEPER_GARGOLMAR" = "watchkeeper gargolmar"
    "NPC_OMOR_THE_UNSCARRED" = "omor the unscarred" 
    "NPC_NAZAN" = "nazan"
    "NPC_VAZRUDEN" = "vazruden the herald"
    
    # Steamvault
    "NPC_HYDROMANCER_THESPIA" = "hydromancer thespia"
    "NPC_MEKGINEER_STEAMRIGGER" = "mekgineer steamrigger"
    "NPC_WARLORD_KALITHRESH" = "warlord kalithresh"
    
    # Blood Furnace
    "NPC_THE_MAKER" = "the maker"
    "NPC_BROGGOK" = "broggok"
    "NPC_KELI_DAN_THE_BREAKER" = "keli'dan the breaker"
    
    # Slave Pens
    "NPC_MENNU_THE_BETRAYER" = "mennu the betrayer"
    "NPC_ROKMAR_THE_CRACKLER" = "rokmar the crackler"
    "NPC_QUAGMIRRAN" = "quagmirran"
    
    # Underbog
    "NPC_HUNGARFEN" = "hungarfen"
    "NPC_GHAZ_AN" = "ghaz'an"
    "NPC_SWAMPLORD_MUSEL_EK" = "swamplord musel'ek"
    "NPC_THE_BLACK_STALKER" = "the black stalker"
    
    # Mana-Tombs
    "NPC_PANDEMONIUS" = "pandemonius"
    "NPC_TAVAROK" = "tavarok"
    "NPC_NEXUS_PRINCE_SHAFFAR" = "nexus-prince shaffar"
    "NPC_YOR" = "yor"
    
    # Magisters' Terrace
    "NPC_SELIN_FIREHEART" = "selin fireheart"
    "NPC_VEXALLUS" = "vexallus"
    "NPC_PRIESTESS_DELRISSA" = "priestess delrissa"
    "NPC_KAEL_THAS_SUNSTRIDER" = "kael'thas sunstrider"
}

# Get all TBC dungeon Actions.cpp files
$TbcActionFiles = @(
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\hellfireramparts\HellfireRampartsActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\steamvault\SteamvaultActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\bloodfurnace\BloodFurnaceActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\slavepens\SlavePensActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\underbog\UnderbogActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\manatombs\ManaTombsActions.cpp",
    "C:\Azerothcore\azerothcore-wotlk\modules\mod-playerbots\src\strategy\dungeons\tbc\magistersterrace\MagistersTerraceActions.cpp"
)

Write-Host "TBC Dungeon Actions.cpp Fix Script - Converting to WotLK patterns"
Write-Host "=================================================================="

foreach ($file in $TbcActionFiles) {
    if (Test-Path $file) {
        Write-Host "Processing: $file"
        $content = Get-Content $file -Raw
        
        # Pattern 1: Replace FindNearestCreature with AI_VALUE2 for boss detection
        $patterns = @(
            # Standard boss detection pattern
            'Unit\* boss = bot->FindNearestCreature\(([^,]+), [^)]+\);',
            'Unit\* ([^=]+) = bot->FindNearestCreature\(([^,]+), [^)]+\);'
        )
        
        foreach ($pattern in $patterns) {
            $matches = [regex]::Matches($content, $pattern)
            foreach ($match in $matches) {
                $npcConstant = $match.Groups[2].Value.Trim()
                $varName = if ($match.Groups.Count -gt 2 -and $match.Groups[1].Value.Trim() -ne "boss") { 
                    $match.Groups[1].Value.Trim() 
                } else { 
                    "boss" 
                }
                
                if ($BossNameMap.ContainsKey($npcConstant)) {
                    $bossName = $BossNameMap[$npcConstant]
                    $replacement = "Unit* $varName = AI_VALUE2(Unit*, `"find target`", `"$bossName`");"
                    $content = $content -replace [regex]::Escape($match.Value), $replacement
                    Write-Host "  Replaced: $npcConstant -> $bossName"
                }
            }
        }
        
        # Pattern 2: Replace manual movement calculations with FleePosition
        $fleePatterns = @{
            # Move away from boss pattern
            'float angle = bot->GetAngle\([^)]+\) \+ M_PI;\s*float x = bot->GetPositionX\(\) \+ cos\(angle\) \* ([^;]+);\s*float y = bot->GetPositionY\(\) \+ sin\(angle\) \* [^;]+;\s*float z = bot->GetPositionZ\(\);\s*return MoveTo\([^)]+\);' = 
                'return FleePosition($1->GetPosition(), $2, 500U);'
        }
        
        foreach ($pattern in $fleePatterns.Keys) {
            $replacement = $fleePatterns[$pattern]
            $content = [regex]::Replace($content, $pattern, $replacement)
        }
        
        # Save the modified content
        Set-Content -Path $file -Value $content -NoNewline
        Write-Host "  File updated successfully"
    }
    else {
        Write-Host "File not found: $file" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Fix completed! All TBC dungeon Actions.cpp files have been updated to use proven WotLK patterns."
Write-Host "Next steps:"
Write-Host "1. Compile to check for any remaining issues"
Write-Host "2. Test in-game to verify functionality"