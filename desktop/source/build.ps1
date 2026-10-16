$ErrorActionPreference = 'Stop'
$source = $PSScriptRoot
$project = Split-Path -Parent $source
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$framework = Split-Path -Parent $compiler
$references = @('System.dll','System.Core.dll','System.Drawing.dll','System.Xaml.dll','System.Web.Extensions.dll') | ForEach-Object { '/reference:' + (Join-Path $framework $_) }
$references += @('WindowsBase.dll','PresentationCore.dll','PresentationFramework.dll') | ForEach-Object { '/reference:' + (Join-Path (Join-Path $framework 'WPF') $_) }
$binary = Join-Path $project 'SD-Karte\3DS Library PC.exe'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $binary) | Out-Null
$sources = @('Library.cs','CiaMetadata.cs','CoverCompression.cs','AutoCovers.cs','AutoCoverTests.cs','Program.cs','Tests.cs','AssemblyInfo.cs') | ForEach-Object { Join-Path $source $_ }
& $compiler /nologo /target:winexe /platform:anycpu /optimize+ /warn:4 /warnaserror+ ('/out:' + $binary) ('/win32manifest:' + (Join-Path $source 'app.manifest')) ('/resource:' + (Join-Path $source 'MainWindow.xaml') + ',MainWindow.xaml') ('/resource:' + (Join-Path $source 'BrandIcon.png') + ',BrandIcon.png') @references @sources
if ($LASTEXITCODE -ne 0) { throw 'Desktop build failed' }
Get-FileHash -LiteralPath $binary | Select-Object Path,Hash
