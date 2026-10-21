param([string]$Reference = 'origin/main')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
& git -C $root fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Aktuelle Remote-Staende konnten nicht geprueft werden. Keine Umsetzung starten.' }
& git -C $root log -1 --format='%h %s'
& git -C $root log -1 --format='%h %s' $Reference
if ($LASTEXITCODE -ne 0) { throw 'Vergleichsbasis fehlt' }
& git -C $root merge-base --is-ancestor $Reference HEAD
if ($LASTEXITCODE -ne 0) { throw 'Neuere oder divergierte Remote-Basis: vor Umsetzung vergleichen. Kein automatischer Merge/Reset.' }
$status = & git -C $root status --porcelain=v1
if ($LASTEXITCODE -ne 0) { throw 'Arbeitsbaum konnte nicht geprueft werden' }
if ($status) {
    $status | Write-Output
    throw 'Lokale Aenderungen erhalten und zuerst zuordnen. Keine automatische Synchronisation.'
}
Write-Output 'Saubere lokale Basis enthaelt die gepruefte Remote-Referenz. Ungepushte Arbeit auf PC 2 bleibt separat zu klaeren.'
