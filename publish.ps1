Param(
    $configuration = "Release",
    $selfContained = $False
)

$publishUrl = "$PSScriptRoot\_publish"
$publishUrl_sc = "$PSScriptRoot\_publish_sc"
$msbuildCommand = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe"

# Clean up
if (Test-Path $publishUrl) { Remove-Item $publishUrl -Recurse -Force }
if (Test-Path $publishUrl_sc) { Remove-Item $publishUrl_sc -Recurse -Force }

# Build the Bridge
. $msbuildCommand .\Bridge\rF2\Bridge.rF2.vcxproj /p:configuration=$configuration /p:platform=win32
. $msbuildCommand .\Bridge\rF2\Bridge.rF2.vcxproj /p:configuration=$configuration /p:platform=x64

# Build the Receiver and Dashboard as cross platform solution
New-Item -ItemType Directory -Path "$publishUrl\Bridge" -Force -Verbose
Copy-Item -Path ".\Bridge\**\bin\**\$configuration\*.dll" -Destination "$publishUrl\Bridge" -Force -Recurse -Verbose
dotnet publish .\Receiver\Receiver\Receiver.csproj -c $configuration -o "$publishUrl\Receiver"
dotnet publish .\Dashboard\Dashboard.sln -c $configuration -o "$publishUrl\Dashboard"

if ($selfContained)
{
    # Build the Receiver and Dashboard as self contained deployment
    New-Item -ItemType Directory -Path "$publishUrl_sc\Bridge" -Force -Verbose
    Copy-Item -Path ".\Bridge\**\bin\**\$configuration\*.dll" -Destination "$publishUrl_sc\Bridge" -Force -Recurse -Verbose
    dotnet publish .\Receiver\Receiver\Receiver.csproj -c $configuration -o "$publishUrl_sc\Receiver" --self-contained -r win10-x64
    dotnet publish .\Dashboard\Dashboard.sln -c $configuration -o "$publishUrl_sc\Dashboard" --self-contained -r win10-x64
}