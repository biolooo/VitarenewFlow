param([Parameter(Mandatory=$true)][string]$Makerom,[Parameter(Mandatory=$true)][string]$Bannertool)
$ErrorActionPreference='Stop'
$app=Join-Path (Split-Path -Parent $PSScriptRoot) 'launcher'
Push-Location $app
try {
 & $Bannertool makebanner -i branding/banner.png -a branding/silence.wav -o branding/banner.bnr
 if($LASTEXITCODE -ne 0){throw 'Banner build failed'}
 & $Makerom -f cia -o VitarenewFlow.cia -rsf branding/application.rsf -target t -elf 3ds-library.elf -icon 3ds-library.smdh -banner branding/banner.bnr
 if($LASTEXITCODE -ne 0){throw 'CIA build failed'}
 Get-FileHash VitarenewFlow.cia
} finally {Pop-Location}
