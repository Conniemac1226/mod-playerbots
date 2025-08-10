Write-Host "=== COMPREHENSIVE TBC DUNGEON PATTERN VERIFICATION ===" -ForegroundColor Cyan
Write-Host ""

# All TBC dungeons
$allTbcDungeons = @(
    "auchenaicrypts",
    "blackmorass",
    "bloodfurnace",
    "escapefromdurnholde",
    "hellfireramparts",
    "magistersterrace",
    "manatombs",
    "mechanar",
    "sethekkhalls",
    "slavepens",
    "steamvault",
    "underbog",
    "shatteredhalls",
    "shadowlabyrinth",
    "arcatraz",
    "botanica"
)

$requiredPatternFiles = @(
    "*Actions.h",
    "*Actions.cpp",
    "*Triggers.h",
    "*Triggers.cpp",
    "*Strategy.h",
    "*Strategy.cpp"
)

$optionalFiles = @(
    "*Multipliers.h",
    "*Multipliers.cpp",
    "*ActionContext.h",
    "*TriggerContext.h"
)

$compliantDungeons = @()
$nonCompliantDungeons = @()

foreach ($dungeon in $allTbcDungeons) {
    Write-Host "Checking: $dungeon" -ForegroundColor Yellow
    
    $path = "modules\mod-playerbots\src\strategy\dungeons\tbc\$dungeon"
    
    if (!(Test-Path $path)) {
        Write-Host "  Directory not found!" -ForegroundColor Red
        $nonCompliantDungeons += $dungeon
        continue
    }
    
    # Check required files
    $hasAllRequired = $true
    foreach ($pattern in $requiredPatternFiles) {
        if (!(Test-Path "$path\$pattern")) {
            Write-Host "  Missing required: $pattern" -ForegroundColor Red
            $hasAllRequired = $false
        }
    }
    
    if (!$hasAllRequired) {
        $nonCompliantDungeons += $dungeon
        Write-Host "  INCOMPLETE PATTERN" -ForegroundColor Red
        continue
    }
    
    # Check Strategy.cpp content
    $strategyFiles = Get-ChildItem "$path\*Strategy.cpp" -ErrorAction SilentlyContinue
    if ($strategyFiles) {
        $content = Get-Content $strategyFiles[0].FullName -Raw
        
        $hasInitTriggers = $content -match "InitTriggers"
        $hasNextAction = $content -match "NextAction::array"
        $hasActionPriorities = $content -match "ACTION_"
        
        if ($hasInitTriggers -and $hasNextAction -and $hasActionPriorities) {
            Write-Host "  PATTERN COMPLIANT" -ForegroundColor Green
            $compliantDungeons += $dungeon
        } else {
            Write-Host "  Missing pattern elements" -ForegroundColor Yellow
            if (!$hasInitTriggers) { Write-Host "    - No InitTriggers" -ForegroundColor Yellow }
            if (!$hasNextAction) { Write-Host "    - No NextAction::array" -ForegroundColor Yellow }
            if (!$hasActionPriorities) { Write-Host "    - No ACTION priorities" -ForegroundColor Yellow }
            $nonCompliantDungeons += $dungeon
        }
    } else {
        Write-Host "  No Strategy.cpp found" -ForegroundColor Red
        $nonCompliantDungeons += $dungeon
    }
}

Write-Host ""
Write-Host "=== SUMMARY ===" -ForegroundColor Cyan
Write-Host "Total TBC Dungeons: $($allTbcDungeons.Count)" -ForegroundColor White
Write-Host "Pattern Compliant: $($compliantDungeons.Count)" -ForegroundColor Green
Write-Host "Non-Compliant: $($nonCompliantDungeons.Count)" -ForegroundColor Red

if ($compliantDungeons.Count -gt 0) {
    Write-Host ""
    Write-Host "Compliant Dungeons:" -ForegroundColor Green
    $compliantDungeons | ForEach-Object { Write-Host "  - $_" -ForegroundColor Green }
}

if ($nonCompliantDungeons.Count -gt 0) {
    Write-Host ""
    Write-Host "Non-Compliant Dungeons:" -ForegroundColor Red
    $nonCompliantDungeons | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
}

Write-Host ""
Write-Host "Recently implemented (should be compliant):" -ForegroundColor Cyan
@("shatteredhalls", "shadowlabyrinth", "arcatraz", "botanica") | ForEach-Object {
    if ($_ -in $compliantDungeons) {
        Write-Host "  - $_ OK" -ForegroundColor Green
    } else {
        Write-Host "  - $_ X" -ForegroundColor Red
    }
}