param([Parameter(Mandatory=$true)][string]$PayloadDirectory,[Parameter(Mandatory=$true)][string]$OutputFile)
$ErrorActionPreference='Stop'
$compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$framework=Split-Path $compiler
$refs=@('System.dll','System.Core.dll','System.Xaml.dll','System.Web.Extensions.dll') | ForEach-Object {'/reference:'+(Join-Path $framework $_)}
$refs+=@('WindowsBase.dll','PresentationCore.dll','PresentationFramework.dll') | ForEach-Object {'/reference:'+(Join-Path $framework ('WPF\'+$_))}
$manifestPath=Join-Path $PayloadDirectory 'manifest.json'
$manifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$resources=@('/resource:'+ $manifestPath +',payload.manifest')
foreach($entry in $manifest.Files){
 $file=Join-Path $PayloadDirectory $entry.Path
 if((Get-FileHash -LiteralPath $file).Hash -ne $entry.SHA256){throw 'Payload hash mismatch'}
 $resources+=('/resource:'+$file+','+$entry.Resource)
}
$sources=@('Installer.cs','Program.cs') | ForEach-Object {Join-Path $PSScriptRoot $_}
$outputPath=[IO.Path]::GetFullPath($OutputFile)
New-Item -ItemType Directory -Force -Path (Split-Path $outputPath)|Out-Null
& $compiler /nologo /target:winexe /platform:anycpu /optimize+ /warn:4 /warnaserror+ ('/out:'+$outputPath) @refs @resources @sources
if($LASTEXITCODE -ne 0){throw 'Setup compilation failed'}
Get-FileHash -LiteralPath $outputPath
