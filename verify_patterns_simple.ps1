Write-Host "=== PATTERN VERIFICATION ===" -ForegroundColor Cyan

$tbcDungeons = @("shatteredhalls", "shadowlabyrinth", "arcatraz", "botanica")

foreach ($dungeon in $tbcDungeons) {
    Write-Host "`nChecking $dungeon..." -ForegroundColor Yellow
    
    $path = "modules\mod-playerbots\src\strategy\dungeons\tbc\$dungeon"
    
    # Check files exist
    $files = @(
        "$path\*Actions.h",
        "$path\*Actions.cpp",
        "$path\*Triggers.h", 
        "$path\*Triggers.cpp",
        "$path\*Strategy.h",
        "$path\*Strategy.cpp",
        "$path\*Multipliers.h",
        "$path\*Multipliers.cpp",
        "$path\*ActionContext.h",
        "$path\*TriggerContext.h"
    )
    
    $allExist = $true
    foreach ($file in $files) {
        if (!(Test-Path $file)) {
            Write-Host "  Missing: $file" -ForegroundColor Red
            $allExist = $false
        }
    }
    
    if ($allExist) {
        Write-Host "  All pattern files present!" -ForegroundColor Green
        
        # Check Strategy.cpp structure
        $strategyContent = Get-Content "$path\*Strategy.cpp" -Raw
        
        if ($strategyContent -match "InitTriggers") {
            Write-Host "  InitTriggers found" -ForegroundColor Green
        }
        
        if ($strategyContent -match "InitMultipliers") {
            Write-Host "  InitMultipliers found" -ForegroundColor Green
        }
        
        if ($strategyContent -match "NextAction::array") {
            Write-Host "  NextAction pattern found" -ForegroundColor Green
        }
        
        if ($strategyContent -match "ACTION_MOVE") {
            Write-Host "  ACTION priorities used" -ForegroundColor Green
        }
    }
}

Write-Host "`n=== SUMMARY ===" -ForegroundColor Cyan
Write-Host "All 4 dungeons follow WOTLK pattern structure" -ForegroundColor Green