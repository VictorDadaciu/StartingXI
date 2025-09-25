$lastWrite = Get-Date -Date "1/01/1970"
$shadersPath = ".\shaders"

$generationPath = "$shadersPath\generated"
New-Item -ItemType Directory -Force -Path $generationPath | out-null
$compilationDateTimePath = "$generationPath\last_compilation_date_time.txt"
if ([System.IO.File]::Exists($compilationDateTimePath)) {
	$lastWriteString = (Get-Content $compilationDateTimePath).Trim()
	$lastWrite = [datetime]::parseexact($lastWriteString, 'dd/MM/yyyy HH:mm:ss', $null)
	Write-Host "Found last compilation datetime: $lastWriteString"
}

$found = 0
foreach ($file in (Get-ChildItem -Path "$shadersPath\*.*" -Include *.vert, *.frag | Where{$_.LastWriteTime -gt $lastWrite})) {
	$found++
	$output = "$generationPath\$($file.BaseName)_$($file.Extension.TrimStart('.')).spv"
	Write-Host "Compiling $file into $output..."
	Start-Process -FilePath "C:\VulkanSDK\1.4.321.1\Bin\glslc.exe" -ArgumentList "$file -o $output"
}

Get-Date -format G | Out-File -FilePath $compilationDateTimePath -NoNewLine
Write-Host "Finished running GLSL -> SPIRV compilation script after compiling $found file(s)"