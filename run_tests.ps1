# NOTE: VIRTUALT_PATH should always reference a directory. The name of the virtualt application is
# computed at runtime and is different for linux and windows.

$env:VIRTUALT_PATH = "C:\Users\John\tools\VirtualT\"

Write-Host "Running vt_process_tests..."
& c:\Users\John\projects\model_t\SuperROMPort\build\Debug\vt_process_tests.exe
Write-Host ""

Write-Host "Running vt_virtual_t_tests..."
& c:\Users\John\projects\model_t\SuperROMPort\build\Debug\vt_virtual_t_tests.exe
Write-Host ""

Write-Host "Running vt_interact_tests..."
& c:\Users\John\projects\model_t\SuperROMPort\build\Debug\vt_interact_tests.exe
Write-Host ""

Write-Host "Running thought_integration_tests..."
& c:\Users\John\projects\model_t\SuperROMPort\build\Debug\thought_integration_tests.exe
Write-Host ""

Write-Host "All tests completed."
