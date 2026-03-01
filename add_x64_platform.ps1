<#
.SYNOPSIS
    Adds x64 platform configurations to the SWG client solution and vcxproj files.
    Preserves all existing Win32 configurations untouched.

.DESCRIPTION
    This script:
    1. Adds x64 platform entries to swg.sln (solution configuration + per-project mappings)
    2. Adds x64 platform configurations to all vcxproj files found under src\
    3. Imports the shared x64.props property sheet for x64 configurations
    
    The Win32 build system is fully preserved. x64 configurations are additive only.

.PARAMETER SolutionDir
    Path to the solution directory (default: src\build\win32)

.PARAMETER DryRun
    If specified, shows what would be changed without modifying files.
#>
param(
    [string]$SolutionDir = "$PSScriptRoot\src\build\win32",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Add-X64ToSolution {
    param([string]$SlnPath)
    
    if (!(Test-Path $SlnPath)) {
        Write-Error "Solution file not found: $SlnPath"
        return
    }
    
    $content = [System.IO.File]::ReadAllText($SlnPath)
    
    if ($content -match "Debug\|x64 = Debug\|x64") {
        Write-Host "  [SKIP] x64 already present in solution" -ForegroundColor Yellow
        return
    }
    
    # Add x64 to SolutionConfigurationPlatforms
    $content = $content -replace `
        "(GlobalSection\(SolutionConfigurationPlatforms\) = preSolution\r?\n)(.*?)(EndGlobalSection)", `
        {
            $prefix = $_.Groups[1].Value
            $body = $_.Groups[2].Value
            $suffix = $_.Groups[3].Value
            
            $newBody = $body
            $newBody += "`t`tDebug|x64 = Debug|x64`r`n"
            $newBody += "`t`tOptimized|x64 = Optimized|x64`r`n"
            $newBody += "`t`tRelease|x64 = Release|x64`r`n"
            
            "$prefix$newBody`t$suffix"
        }
    
    # For each project GUID, add x64 configuration mappings
    # Pattern: {GUID}.Config|Win32.ActiveCfg = Config|Win32
    $guidPattern = '\{[A-Fa-f0-9\-]+\}'
    $configs = @("Debug", "Optimized", "Release")
    
    $lines = $content -split "`r?`n"
    $newLines = [System.Collections.Generic.List[string]]::new()
    $processedGuids = @{}
    
    foreach ($line in $lines) {
        $newLines.Add($line)
        
        # After each project's Release|Win32.Build.0 line, add x64 mappings
        if ($line -match "^\s+($guidPattern)\.Release\|Win32\.Build\.0\s*=\s*Release\|Win32") {
            $guid = $Matches[1]
            if (!$processedGuids.ContainsKey($guid)) {
                $processedGuids[$guid] = $true
                foreach ($cfg in $configs) {
                    $newLines.Add("`t`t$guid.$cfg|x64.ActiveCfg = $cfg|x64")
                    $newLines.Add("`t`t$guid.$cfg|x64.Build.0 = $cfg|x64")
                }
            }
        }
    }
    
    $result = $newLines -join "`r`n"
    
    if ($DryRun) {
        Write-Host "  [DRY RUN] Would update $SlnPath" -ForegroundColor Cyan
    } else {
        [System.IO.File]::WriteAllText($SlnPath, $result)
        Write-Host "  [OK] Updated $SlnPath" -ForegroundColor Green
    }
}

function Add-X64ToVcxproj {
    param([string]$VcxprojPath)
    
    $xml = [xml](Get-Content $VcxprojPath -Raw)
    $ns = "http://schemas.microsoft.com/developer/msbuild/2003"
    $nsmgr = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $nsmgr.AddNamespace("ms", $ns)
    
    # Check if x64 already exists
    $existing = $xml.SelectNodes("//ms:ProjectConfiguration[ms:Platform='x64']", $nsmgr)
    if ($existing.Count -gt 0) {
        Write-Host "  [SKIP] x64 already present in $($VcxprojPath | Split-Path -Leaf)" -ForegroundColor Yellow
        return
    }
    
    # Determine relative path to solution dir for x64.props import
    $vcxDir = (Resolve-Path (Split-Path $VcxprojPath -Parent)).Path
    $slnDir = (Resolve-Path $SolutionDir).Path
    
    $vcxUri = [System.Uri]::new("$vcxDir\")
    $slnUri = [System.Uri]::new("$slnDir\")
    $relPath = $vcxUri.MakeRelativeUri($slnUri).ToString() -replace '/', '\'
    $relPath = $relPath.TrimEnd('\')
    $x64PropsRel = "$relPath\x64.props"
    
    # 1. Add ProjectConfiguration entries for x64
    $projConfigs = $xml.SelectSingleNode("//ms:ItemGroup[@Label='ProjectConfigurations']", $nsmgr)
    if ($projConfigs) {
        foreach ($cfg in @("Debug", "Optimized", "Release")) {
            $newConfig = $xml.CreateElement("ProjectConfiguration", $ns)
            $newConfig.SetAttribute("Include", "$cfg|x64")
            
            $cfgElem = $xml.CreateElement("Configuration", $ns)
            $cfgElem.InnerText = $cfg
            $newConfig.AppendChild($cfgElem) | Out-Null
            
            $platElem = $xml.CreateElement("Platform", $ns)
            $platElem.InnerText = "x64"
            $newConfig.AppendChild($platElem) | Out-Null
            
            $projConfigs.AppendChild($newConfig) | Out-Null
        }
    }
    
    # 2. Clone Configuration PropertyGroups for x64
    $configPGs = $xml.SelectNodes("//ms:PropertyGroup[@Label='Configuration']", $nsmgr)
    foreach ($pg in $configPGs) {
        $cond = $pg.GetAttribute("Condition")
        if ($cond -match "Win32") {
            $newPG = $pg.Clone()
            $newCond = $cond -replace "Win32", "x64"
            $newPG.SetAttribute("Condition", $newCond)
            $pg.ParentNode.InsertAfter($newPG, $pg) | Out-Null
        }
    }
    
    # 3. Clone PropertySheet ImportGroups for x64 and add x64.props import
    $importGroups = $xml.SelectNodes("//ms:ImportGroup[@Label='PropertySheets']", $nsmgr)
    foreach ($ig in $importGroups) {
        $cond = $ig.GetAttribute("Condition")
        if ($cond -match "Win32") {
            $newIG = $ig.Clone()
            $newCond = $cond -replace "Win32", "x64"
            $newIG.SetAttribute("Condition", $newCond)
            
            # Add x64.props import
            $propsImport = $xml.CreateElement("Import", $ns)
            $propsImport.SetAttribute("Project", "`$(SolutionDir)x64.props")
            $newIG.AppendChild($propsImport) | Out-Null
            
            $ig.ParentNode.InsertAfter($newIG, $ig) | Out-Null
        }
    }
    
    # 4. Clone output directory PropertyGroups for x64
    $outDirPGs = $xml.SelectNodes("//ms:PropertyGroup[ms:OutDir and not(@Label)]", $nsmgr)
    if ($outDirPGs.Count -eq 0) {
        $outDirPGs = $xml.SelectNodes("//ms:PropertyGroup[ms:OutDir]", $nsmgr)
    }
    foreach ($pg in $outDirPGs) {
        $cond = $pg.GetAttribute("Condition")
        if ($cond -match "Win32") {
            $newPG = $pg.Clone()
            $newCond = $cond -replace "Win32", "x64"
            $newPG.SetAttribute("Condition", $newCond)
            
            # Update output paths from win32 to x64
            $outDir = $newPG.SelectSingleNode("ms:OutDir", $nsmgr)
            if ($outDir) { $outDir.InnerText = $outDir.InnerText -replace "\\win32\\", "\x64\" }
            $intDir = $newPG.SelectSingleNode("ms:IntDir", $nsmgr)
            if ($intDir) { $intDir.InnerText = $intDir.InnerText -replace "\\win32\\", "\x64\" }
            
            $pg.ParentNode.InsertAfter($newPG, $pg) | Out-Null
        }
    }
    
    # 5. Clone ItemDefinitionGroups for x64
    $itemDefGroups = $xml.SelectNodes("//ms:ItemDefinitionGroup[@Condition]", $nsmgr)
    foreach ($idg in $itemDefGroups) {
        $cond = $idg.GetAttribute("Condition")
        if ($cond -match "Win32") {
            $newIDG = $idg.Clone()
            $newCond = $cond -replace "Win32", "x64"
            $newIDG.SetAttribute("Condition", $newCond)
            
            # Remove STLport from include directories
            $clCompile = $newIDG.SelectSingleNode("ms:ClCompile", $nsmgr)
            if ($clCompile) {
                $addInc = $clCompile.SelectSingleNode("ms:AdditionalIncludeDirectories", $nsmgr)
                if ($addInc) {
                    $addInc.InnerText = ($addInc.InnerText -replace '[^;]*stlport453[^;]*;?', '').TrimEnd(';')
                }
                
                # Remove _USE_32BIT_TIME_T and add WIN64
                $ppDefs = $clCompile.SelectSingleNode("ms:PreprocessorDefinitions", $nsmgr)
                if ($ppDefs) {
                    $defs = $ppDefs.InnerText
                    $defs = $defs -replace '_USE_32BIT_TIME_T=1;?', ''
                    if ($defs -notmatch 'WIN64') {
                        $defs = $defs -replace 'WIN32;', 'WIN32;WIN64;'
                    }
                    $ppDefs.InnerText = $defs
                }
            }
            
            # Remove x86-only libraries from x64 link dependencies
            $x86OnlyLibs = @(
                'stlport_vc6_static.lib',
                'stlport_vc6_stldebug_static.lib',
                'stlport_vc7_static.lib',
                'stlport_vc71_static.lib',
                'vivoxSharedWrapper_release.lib',
                'vivoxSharedWrapper_Release.lib',
                'vivoxplatform.lib',
                'vivoxsdk.lib',
                'mss32.lib',
                'libpcre.a'
            )
            
            $link = $newIDG.SelectSingleNode("ms:Link", $nsmgr)
            if ($link) {
                $addDeps = $link.SelectSingleNode("ms:AdditionalDependencies", $nsmgr)
                if ($addDeps) {
                    $deps = $addDeps.InnerText -split ';'
                    $deps = $deps | Where-Object { $_ -and ($x86OnlyLibs -notcontains $_) }
                    $addDeps.InnerText = ($deps -join ';')
                }
            }

            # Update Lib/Link target machine
            $lib = $newIDG.SelectSingleNode("ms:Lib", $nsmgr)
            if ($lib) {
                $targetMachine = $xml.CreateElement("TargetMachine", $ns)
                $targetMachine.InnerText = "MachineX64"
                $lib.AppendChild($targetMachine) | Out-Null
            }
            $link = $newIDG.SelectSingleNode("ms:Link", $nsmgr)
            if ($link) {
                $existingTM = $link.SelectSingleNode("ms:TargetMachine", $nsmgr)
                if ($existingTM) {
                    $existingTM.InnerText = "MachineX64"
                } else {
                    $targetMachine = $xml.CreateElement("TargetMachine", $ns)
                    $targetMachine.InnerText = "MachineX64"
                    $link.AppendChild($targetMachine) | Out-Null
                }
                
                # Update library paths from win32 to x64 where applicable
                $addLibDirs = $link.SelectSingleNode("ms:AdditionalLibraryDirectories", $nsmgr)
                if ($addLibDirs) {
                    $addLibDirs.InnerText = $addLibDirs.InnerText -replace "\\win32\\", "\x64\"
                }
            }
            
            $idg.ParentNode.InsertAfter($newIDG, $idg) | Out-Null
        }
    }
    
    # 6. Add x64 conditions to per-file settings (PCH create, optimization overrides)
    $clCompileItems = $xml.SelectNodes("//ms:ItemGroup/ms:ClCompile", $nsmgr)
    foreach ($item in $clCompileItems) {
        $childNodes = @($item.ChildNodes | Where-Object { $_.GetAttribute -and $_.GetAttribute("Condition") -match "Win32" })
        foreach ($child in $childNodes) {
            $cond = $child.GetAttribute("Condition")
            $newChild = $child.Clone()
            $newCond = $cond -replace "Win32", "x64"
            $newChild.SetAttribute("Condition", $newCond)
            $item.InsertAfter($newChild, $child) | Out-Null
        }
    }
    
    if ($DryRun) {
        Write-Host "  [DRY RUN] Would update $($VcxprojPath | Split-Path -Leaf)" -ForegroundColor Cyan
    } else {
        $settings = New-Object System.Xml.XmlWriterSettings
        $settings.Indent = $true
        $settings.IndentChars = "  "
        $settings.Encoding = [System.Text.UTF8Encoding]::new($false)
        $writer = [System.Xml.XmlWriter]::Create($VcxprojPath, $settings)
        $xml.Save($writer)
        $writer.Close()
        Write-Host "  [OK] Updated $($VcxprojPath | Split-Path -Leaf)" -ForegroundColor Green
    }
}

# Main execution
Write-Host "=== SWG x64 Platform Migration Tool ===" -ForegroundColor White
Write-Host ""

$slnPath = Join-Path $SolutionDir "swg.sln"

# Step 1: Update solution file
Write-Host "Step 1: Updating solution file..." -ForegroundColor White
Add-X64ToSolution -SlnPath $slnPath

# Step 2: Find and update all vcxproj files
Write-Host ""
Write-Host "Step 2: Updating vcxproj files..." -ForegroundColor White

$srcRoot = "$PSScriptRoot\src"
$vcxprojFiles = Get-ChildItem -Path $srcRoot -Filter "*.vcxproj" -Recurse | Where-Object { $_.FullName -notmatch "\\compile\\" }

$total = $vcxprojFiles.Count
$current = 0

foreach ($file in $vcxprojFiles) {
    $current++
    Write-Host "  [$current/$total] Processing $($file.Name)..." -ForegroundColor Gray
    try {
        Add-X64ToVcxproj -VcxprojPath $file.FullName
    } catch {
        Write-Host "  [ERROR] Failed to process $($file.Name): $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "=== Done ===" -ForegroundColor White
Write-Host "x64 configurations added. Win32 configurations preserved." -ForegroundColor Green
Write-Host ""
Write-Host "To build x64:" -ForegroundColor Yellow
Write-Host '  msbuild swg.sln /p:Configuration=Release /p:Platform=x64' -ForegroundColor Yellow
Write-Host ""
Write-Host "To build Win32 (unchanged):" -ForegroundColor Yellow
Write-Host '  msbuild swg.sln /p:Configuration=Release /p:Platform=Win32' -ForegroundColor Yellow
