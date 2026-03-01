$ns = "http://schemas.microsoft.com/developer/msbuild/2003"
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

$srcRoot = "$PSScriptRoot\src"
$vcxprojFiles = Get-ChildItem -Path $srcRoot -Filter "*.vcxproj" -Recurse | Where-Object { $_.FullName -notmatch "\\compile\\" }
$modified = 0

foreach ($file in $vcxprojFiles) {
    $xml = [xml](Get-Content $file.FullName -Raw)
    $nsmgr = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $nsmgr.AddNamespace("ms", $ns)
    
    $changed = $false
    $itemDefGroups = $xml.SelectNodes("//ms:ItemDefinitionGroup[@Condition]", $nsmgr)
    foreach ($idg in $itemDefGroups) {
        $cond = $idg.GetAttribute("Condition")
        if ($cond -match "x64") {
            $link = $idg.SelectSingleNode("ms:Link", $nsmgr)
            if ($link) {
                $addDeps = $link.SelectSingleNode("ms:AdditionalDependencies", $nsmgr)
                if ($addDeps) {
                    $deps = $addDeps.InnerText -split ';'
                    $newDeps = $deps | Where-Object { $_ -and ($x86OnlyLibs -notcontains $_) }
                    $newText = ($newDeps -join ';')
                    if ($newText -ne $addDeps.InnerText) {
                        $addDeps.InnerText = $newText
                        $changed = $true
                    }
                }
            }
        }
    }
    
    if ($changed) {
        $settings = New-Object System.Xml.XmlWriterSettings
        $settings.Indent = $true
        $settings.IndentChars = "  "
        $settings.Encoding = [System.Text.UTF8Encoding]::new($false)
        $writer = [System.Xml.XmlWriter]::Create($file.FullName, $settings)
        $xml.Save($writer)
        $writer.Close()
        $modified++
        Write-Host "  [OK] Cleaned x86 libs from x64 config: $($file.Name)" -ForegroundColor Green
    }
}
Write-Host "Modified $modified files" -ForegroundColor Cyan
