# Find Vulkan
find_package(Vulkan)
if (NOT CROSS_COMPILE_ARM AND (Vulkan_FOUND OR WIN32))
	# We have Vulkan/DirectX so we can compile TestFramework
	set(TEST_FRAMEWORK_AVAILABLE TRUE)

	# Root
	set(TEST_FRAMEWORK_ROOT ${PHYSICS_REPO_ROOT}/TestFramework)

	# Source files
	set(TEST_FRAMEWORK_SRC_FILES
		${TEST_FRAMEWORK_ROOT}/Application/Application.cpp
		${TEST_FRAMEWORK_ROOT}/Application/Application.h
		${TEST_FRAMEWORK_ROOT}/Application/DebugUI.cpp
		${TEST_FRAMEWORK_ROOT}/Application/DebugUI.h
		${TEST_FRAMEWORK_ROOT}/Application/EntryPoint.h
		${TEST_FRAMEWORK_ROOT}/External/Perlin.cpp
		${TEST_FRAMEWORK_ROOT}/External/Perlin.h
		${TEST_FRAMEWORK_ROOT}/External/stb_truetype.h
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
		${TEST_FRAMEWORK_ROOT}/Input/Keyboard.h
		${TEST_FRAMEWORK_ROOT}/Input/Mouse.h
		${TEST_FRAMEWORK_ROOT}/Renderer/DebugRendererImp.cpp
		${TEST_FRAMEWORK_ROOT}/Renderer/DebugRendererImp.h
		${TEST_FRAMEWORK_ROOT}/Renderer/Font.cpp
		${TEST_FRAMEWORK_ROOT}/Renderer/Font.h
		${TEST_FRAMEWORK_ROOT}/Renderer/Frustum.h
		${TEST_FRAMEWORK_ROOT}/Renderer/PipelineState.h
		${TEST_FRAMEWORK_ROOT}/Renderer/PixelShader.h
		${TEST_FRAMEWORK_ROOT}/Renderer/Renderer.cpp
		${TEST_FRAMEWORK_ROOT}/Renderer/Renderer.h
		${TEST_FRAMEWORK_ROOT}/Renderer/RenderInstances.h
		${TEST_FRAMEWORK_ROOT}/Renderer/RenderPrimitive.cpp
		${TEST_FRAMEWORK_ROOT}/Renderer/RenderPrimitive.h
		${TEST_FRAMEWORK_ROOT}/Renderer/Texture.h
		${TEST_FRAMEWORK_ROOT}/Renderer/VertexShader.h
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
		${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindow.h
	)

	if (WIN32)
		# Windows source files
		set(TEST_FRAMEWORK_SRC_FILES
			${TEST_FRAMEWORK_SRC_FILES}
			${TEST_FRAMEWORK_ROOT}/Input/Win/KeyboardWin.cpp
			${TEST_FRAMEWORK_ROOT}/Input/Win/KeyboardWin.h
			${TEST_FRAMEWORK_ROOT}/Input/Win/MouseWin.cpp
			${TEST_FRAMEWORK_ROOT}/Input/Win/MouseWin.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/ConstantBufferDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/ConstantBufferDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/CommandQueueDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/DescriptorHeapDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/FatalErrorIfFailedDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/FatalErrorIfFailedDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/PipelineStateDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/PipelineStateDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/PixelShaderDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RendererDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RendererDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RenderInstancesDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RenderInstancesDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RenderPrimitiveDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/RenderPrimitiveDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/TextureDX12.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/TextureDX12.h
			${TEST_FRAMEWORK_ROOT}/Renderer/DX12/VertexShaderDX12.h
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowWin.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowWin.h
		)

		# All shaders
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VertexConstants.h
		)

		# HLSL vertex shaders
		set(TEST_FRAMEWORK_HLSL_VERTEX_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/FontVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/LineVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIVertexShader.hlsl
		)
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_HLSL_VERTEX_SHADERS})
		set_source_files_properties(${TEST_FRAMEWORK_HLSL_VERTEX_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T vs_5_0")

		# HLSL pixel shaders
		set(TEST_FRAMEWORK_HLSL_PIXEL_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/FontPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/LinePixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TrianglePixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShaderUntextured.hlsl
		)
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_HLSL_PIXEL_SHADERS})
		set_source_files_properties(${TEST_FRAMEWORK_HLSL_PIXEL_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T ps_5_0")
	endif()

	if (LINUX)
		# Linux source files
		set(TEST_FRAMEWORK_SRC_FILES
			${TEST_FRAMEWORK_SRC_FILES}
			${TEST_FRAMEWORK_ROOT}/Input/Linux/KeyboardLinux.cpp
			${TEST_FRAMEWORK_ROOT}/Input/Linux/KeyboardLinux.h
			${TEST_FRAMEWORK_ROOT}/Input/Linux/MouseLinux.cpp
			${TEST_FRAMEWORK_ROOT}/Input/Linux/MouseLinux.h
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowLinux.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowLinux.h
		)
	endif()
		
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
		# macOS source files
		set(TEST_FRAMEWORK_SRC_FILES
			${TEST_FRAMEWORK_SRC_FILES}
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/KeyboardMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/KeyboardMacOS.h
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/MouseMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/MouseMacOS.h
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowMacOS.h
		)
	endif()

	# Include the Vulkan library
	if (Vulkan_FOUND)
		# Vulkan source files
		set(TEST_FRAMEWORK_SRC_FILES
			${TEST_FRAMEWORK_SRC_FILES}
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/BufferVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/ConstantBufferVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/ConstantBufferVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/FatalErrorIfFailedVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/FatalErrorIfFailedVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/PipelineStateVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/PipelineStateVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/PixelShaderVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RendererVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RendererVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RenderInstancesVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RenderInstancesVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RenderPrimitiveVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/RenderPrimitiveVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/TextureVK.cpp
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/TextureVK.h
			${TEST_FRAMEWORK_ROOT}/Renderer/VK/VertexShaderVK.h
		)

		# GLSL headers
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS
			${TEST_FRAMEWORK_SRC_FILES_SHADERS}
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VertexConstantsVK.h
		)

		# GLSL shaders
		set(TEST_FRAMEWORK_GLSL_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/FontVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/LineVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/FontPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/LinePixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TriangleDepthPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/TrianglePixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/UIPixelShaderUntextured.frag
		)

		# Compile GLSL shaders
		foreach(SHADER ${TEST_FRAMEWORK_GLSL_SHADERS})
			set(SPV_SHADER ${SHADER}.spv)
			add_custom_command(OUTPUT ${SPV_SHADER}
				COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_SHADER}
				DEPENDS ${SHADER}
				COMMENT "Compiling ${SHADER}")
			list(APPEND TEST_FRAMEWORK_SPV_SHADERS ${SPV_SHADER})
		endforeach()
	endif()

	# Group source files
	source_group(TREE ${TEST_FRAMEWORK_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES})

	# Group shader files
	source_group(TREE ${PHYSICS_REPO_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_GLSL_SHADERS} ${TEST_FRAMEWORK_SPV_SHADERS})

	# Create TestFramework lib
	add_library(TestFramework STATIC ${TEST_FRAMEWORK_SRC_FILES} ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_SPV_SHADERS})
	target_include_directories(TestFramework PUBLIC ${TEST_FRAMEWORK_ROOT})
	target_precompile_headers(TestFramework PUBLIC ${TEST_FRAMEWORK_ROOT}/TestFramework.h)

	if (Vulkan_FOUND)
		# Vulkan configuration
		target_include_directories(TestFramework PUBLIC ${Vulkan_INCLUDE_DIRS})
		target_link_libraries(TestFramework LINK_PUBLIC Jolt ${Vulkan_LIBRARIES})
		if (JPH_ENABLE_VULKAN)
			target_compile_definitions(TestFramework PRIVATE JPH_ENABLE_VULKAN)
		endif()
	endif()
	if (WIN32)
		# Windows configuration
		target_link_libraries(TestFramework LINK_PUBLIC Jolt dxguid.lib dinput8.lib dxgi.lib d3d12.lib d3dcompiler.lib shcore.lib)
		target_compile_definitions(TestFramework PRIVATE JPH_ENABLE_DIRECTX)
	endif()
	if (LINUX)
		# Linux configuration
		target_link_libraries(TestFramework LINK_PUBLIC Jolt X11)
	endif()
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
		# macOS configuration
		target_link_libraries(TestFramework LINK_PUBLIC Jolt "-framework Cocoa -framework Metal -framework MetalKit -framework GameController")
		
		# Ignore PCH files for .mm files
		foreach(SRC_FILE ${TEST_FRAMEWORK_SRC_FILES})
			if (SRC_FILE MATCHES "\.mm")
				set_source_files_properties(${SRC_FILE} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
			endif()
		endforeach()
	endif()
else()
	# No graphics framework found
	set(TEST_FRAMEWORK_AVAILABLE FALSE)
endif()
