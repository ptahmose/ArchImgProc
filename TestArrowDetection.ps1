$OutputDirBase = "W:\Data\ArrowDetect\"
$Exename = "D:\DEV\GitHub\ArchImgProc\x64\Debug\LineSegmenterTest.exe"
$files = Get-ChildItem "W:\Data\Archery-Targets" -Filter *.jpg 
for ($i=0;$i -lt $files.Count;$i++){
    $OutDir = $OutputDirBase + [io.path]::GetFileNameWithoutExtension($files[$i]) +"\"
    Write-Host $OutDir
    If(!(test-path $OutDir))
    {
        New-Item -ItemType Directory -Force -path $OutDir
    }

    Write-Host $files[$i].FullName
    &$Exename -s $files[$i].FullName -o $OutDir -p output
    
}