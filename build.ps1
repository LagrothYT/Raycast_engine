g++ main.cpp -o game -lraylib -lopengl32 -lgdi32 -lwinmm

if ($?) {
    Write-Host "Build Successful!" -ForegroundColor Green
    $file = Get-Item "game.exe"
    Write-Host "File Size: $($file.Length) bytes" -ForegroundColor Green
    ./game.exe
} else {
    Write-Error "Build Failed with exit code $LASTEXITCODE"
}