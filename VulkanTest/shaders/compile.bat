
for /r %%i in (*.vert) do (

echo %%~ni
C:/VulkanSDK/1.2.182.0/Bin32/glslc.exe %%~ni.vert -o %%~nivert.spv
C:/VulkanSDK/1.2.182.0/Bin32/glslc.exe %%~ni.frag -o %%~nifrag.spv
)

::C:/VulkanSDK/1.2.182.0/Bin32/glslc.exe shader.vert -o vert.spv
::C:/VulkanSDK/1.2.182.0/Bin32/glslc.exe shader2.vert -o vert2.spv
::C:/VulkanSDK/1.2.182.0/Bin32/glslc.exe shader.frag -o frag.spv
pause