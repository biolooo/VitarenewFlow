param(
    [string]$Destination = (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Code\vitarenew-flow'),
    [string]$Branch = 'codex/pc2-handoff'
)
$ErrorActionPreference = 'Stop'
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw 'Git fehlt. Git for Windows installieren, PowerShell neu oeffnen und erneut starten.'
}
if (Test-Path -LiteralPath $Destination) {
    throw "Ziel existiert bereits: $Destination. Bestehende Arbeit zuerst pruefen; nichts wurde ersetzt."
}
$parent = Split-Path -Parent ([IO.Path]::GetFullPath($Destination))
New-Item -ItemType Directory -Path $parent -Force | Out-Null
& git clone --single-branch --branch $Branch -- https://github.com/biolooo/VitarenewFlow.git $Destination
if ($LASTEXITCODE -ne 0) { throw 'Klonen fehlgeschlagen. Eventuell teilweise angelegten Ordner vor erneutem Versuch pruefen.' }
$release = Join-Path $Destination 'docs\updates\0.12.33'
$manifest = Get-Content -LiteralPath (Join-Path $release 'SHA256.json') -Raw | ConvertFrom-Json
foreach ($entry in $manifest.PSObject.Properties) {
    if ([IO.Path]::GetFileName($entry.Name) -ne $entry.Name -or $entry.Name -match '[/\\]') { throw 'Ungueltiger Manifestpfad' }
    $actual = (Get-FileHash -LiteralPath (Join-Path $release $entry.Name) -Algorithm SHA256).Hash
    if ($actual -ne $entry.Value) { throw "Releasehash stimmt nicht: $($entry.Name)" }
}
& git -C $Destination status --short
if ($LASTEXITCODE -ne 0) { throw 'Git-Status konnte nicht gelesen werden' }
Write-Host "Projekt bereit: $Destination"
Write-Host 'Alle acht .33-Releasehashes geprueft. Diesen Ordner in Codex als lokales Projekt oeffnen.'
Write-Host 'Zuerst AGENTS.md und docs/HANDOFF.md lesen. GitHub-Anmeldung fuer spaetere Pushes separat einrichten.'
Write-Host 'Buildwerkzeuge sind noch nicht eingerichtet: WSL/SDK und Windows .NET/WPF auf diesem PC pruefen.'
