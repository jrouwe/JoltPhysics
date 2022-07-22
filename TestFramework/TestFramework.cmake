# Root
set(TEST_FRAMEWORK_ROOT ${PHYSICS_REPO_ROOT}/TestFramework)

# Source files
set(TEST_FRAMEWORK_SRC_FILES
	${TEST_FRAMEWORK_ROOT}/Application/Application.cpp
	${TEST_FRAMEWORK_ROOT}/Application/Application.h
	${TEST_FRAMEWORK_ROOT}/Application/DebugUI.cpp
	${TEST_FRAMEWORK_ROOT}/Application/DebugUI.h
	${TEST_FRAMEWORK_ROOT}/Application/EntryPoint.h
	${TEST_FRAMEWORK_ROOT}/Image/BlitSurface.cpp
	${TEST_FRAMEWORK_ROOT}/Image/BlitSurface.h
	${TEST_FRAMEWORK_ROOT}/Image/LoadBMP.cpp
	${TEST_FRAMEWORK_ROOT}/Image/LoadBMP.h
	${TEST_FRAMEWORK_ROOT}/Image/LoadTGA.cpp
	${TEST_FRAMEWORK_ROOT}/Image/LoadTGA.h
	${TEST_FRAMEWORK_ROOT}/Image/Surface.cpp
	${TEST_FRAMEWORK_ROOT}/Image/Surface.h
	${TEST_FRAMEWORK_ROOT}/Image/ZoomImage.cpp
	${TEST_FRAMEWORK_ROOT}/Image/ZoomImage.h
	${TEST_FRAMEWORK_ROOT}/Input/Keyboard.cpp
	${TEST_FRAMEWORK_ROOT}/Input/Keyboard.h
	${TEST_FRAMEWORK_ROOT}/Input/Mouse.cpp
	${TEST_FRAMEWORK_ROOT}/Input/Mouse.h
	${TEST_FRAMEWORK_ROOT}/Math/Perlin.cpp
	${TEST_FRAMEWORK_ROOT}/Math/Perlin.h
	${TEST_FRAMEWORK_ROOT}/Renderer/ConstantBuffer.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/ConstantBuffer.h
	${TEST_FRAMEWORK_ROOT}/Renderer/CommandQueue.h
	${TEST_FRAMEWORK_ROOT}/Renderer/DescriptorHeap.h
	${TEST_FRAMEWORK_ROOT}/Renderer/FatalErrorIfFailed.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/FatalErrorIfFailed.h
	${TEST_FRAMEWORK_ROOT}/Renderer/Font.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/Font.h
	${TEST_FRAMEWORK_ROOT}/Renderer/Frustum.h
	${TEST_FRAMEWORK_ROOT}/Renderer/PipelineState.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/PipelineState.h
	${TEST_FRAMEWORK_ROOT}/Renderer/Renderer.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/Renderer.h
	${TEST_FRAMEWORK_ROOT}/Renderer/RenderInstances.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/RenderInstances.h
	${TEST_FRAMEWORK_ROOT}/Renderer/RenderPrimitive.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/RenderPrimitive.h
	${TEST_FRAMEWORK_ROOT}/Renderer/Texture.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/Texture.h
	${TEST_FRAMEWORK_ROOT}/Renderer/DebugRendererImp.cpp
	${TEST_FRAMEWORK_ROOT}/Renderer/DebugRendererImp.h
	${TEST_FRAMEWORK_ROOT}/TestFramework.cmake
	${TEST_FRAMEWORK_ROOT}/TestFramework.h
	${TEST_FRAMEWORK_ROOT}/UI/UIAnimation.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIAnimation.h
	${TEST_FRAMEWORK_ROOT}/UI/UIAnimationSlide.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIAnimationSlide.h
	${TEST_FRAMEWORK_ROOT}/UI/UIButton.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIButton.h
	${TEST_FRAMEWORK_ROOT}/UI/UICheckBox.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UICheckBox.h
	${TEST_FRAMEWORK_ROOT}/UI/UIComboBox.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIComboBox.h
	${TEST_FRAMEWORK_ROOT}/UI/UIElement.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIElement.h
	${TEST_FRAMEWORK_ROOT}/UI/UIEventListener.h
	${TEST_FRAMEWORK_ROOT}/UI/UIHorizontalStack.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIHorizontalStack.h
	${TEST_FRAMEWORK_ROOT}/UI/UIImage.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIImage.h
	${TEST_FRAMEWORK_ROOT}/UI/UIManager.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIManager.h
	${TEST_FRAMEWORK_ROOT}/UI/UISlider.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UISlider.h
	${TEST_FRAMEWORK_ROOT}/UI/UIStaticText.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIStaticText.h
	${TEST_FRAMEWORK_ROOT}/UI/UITextButton.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UITextButton.h
	${TEST_FRAMEWORK_ROOT}/UI/UITexturedQuad.h
	${TEST_FRAMEWORK_ROOT}/UI/UIVerticalStack.cpp
	${TEST_FRAMEWORK_ROOT}/UI/UIVerticalStack.h
	${TEST_FRAMEWORK_ROOT}/Utils/CustomMemoryHook.cpp
	${TEST_FRAMEWORK_ROOT}/Utils/CustomMemoryHook.h
	${TEST_FRAMEWORK_ROOT}/Utils/Log.cpp
	${TEST_FRAMEWORK_ROOT}/Utils/Log.h
	${TEST_FRAMEWORK_ROOT}/Utils/ReadData.cpp
	${TEST_FRAMEWORK_ROOT}/Utils/ReadData.h
)

# Group source files
source_group(TREE ${TEST_FRAMEWORK_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES})

# All shaders
set(TEST_FRAMEWORK_SRC_FILES_SHADERS
	${PHYSICS_REPO_ROOT}/Assets/Shaders/VertexConstants.h
)

# Vertex shaders
set(TEST_FRAMEWORK_VERTEX_SHADERS
	${PHYSICS_REPO_ROOT}/Assets/Shaders/FontVertexShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/LineVertexShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthVertexShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleVertexShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/UIVertexShader.hlsl
)
set(TEST_FRAMEWORK_SRC_FILES_SHADERS ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_VERTEX_SHADERS})
set_source_files_properties(${TEST_FRAMEWORK_VERTEX_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T vs_5_0")

# Pixel shaders
set(TEST_FRAMEWORK_PIXEL_SHADERS
	${PHYSICS_REPO_ROOT}/Assets/Shaders/FontPixelShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/LinePixelShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthPixelShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/TrianglePixelShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShader.hlsl
	${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShaderUntextured.hlsl
)
set(TEST_FRAMEWORK_SRC_FILES_SHADERS ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_PIXEL_SHADERS})
set_source_files_properties(${TEST_FRAMEWORK_PIXEL_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T ps_5_0")

# Group shader files
source_group(TREE ${PHYSICS_REPO_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES_SHADERS})

# Create TestFramework lib
add_library(TestFramework STATIC ${TEST_FRAMEWORK_SRC_FILES} ${TEST_FRAMEWORK_SRC_FILES_SHADERS})
target_include_directories(TestFramework PUBLIC ${TEST_FRAMEWORK_ROOT})
target_link_libraries(TestFramework LINK_PUBLIC Jolt dxguid.lib dinput8.lib dxgi.lib d3d12.lib d3dcompiler.lib)
target_precompile_headers(TestFramework PUBLIC ${TEST_FRAMEWORK_ROOT}/TestFramework.h)
