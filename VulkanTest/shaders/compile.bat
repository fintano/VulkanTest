
for /r %%i in (*.vert) do (
echo %%~ni
%VULKAN_SDK%\Bin\glslc.exe %%~ni.vert -o %%~nivert.spv
)

for /r %%i in (*.frag) do (
echo %%~ni
%VULKAN_SDK%\Bin\glslc.exe %%~ni.frag -o %%~nifrag.spv
)

pause