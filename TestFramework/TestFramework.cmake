# Find Vulkan
find_package(Vulkan)
if (NOT CROSS_COMPILE_ARM AND (Vulkan_FOUND OR WIN32 OR ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")))
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
		${TEST_FRAMEWORK_ROOT}/Utils/AssetStream.h
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
			${TEST_FRAMEWORK_ROOT}/Utils/AssetStream.cpp
			${TEST_FRAMEWORK_ROOT}/Utils/Log.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowWin.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowWin.h
		)

		# HLSL vertex shaders
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/VertexConstants.h
		)
		set(TEST_FRAMEWORK_HLSL_VERTEX_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/FontVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/LineVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/TriangleDepthVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/TriangleVertexShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/UIVertexShader.hlsl
		)
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_HLSL_VERTEX_SHADERS})
		set_source_files_properties(${TEST_FRAMEWORK_HLSL_VERTEX_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T vs_5_0")

		# HLSL pixel shaders
		set(TEST_FRAMEWORK_HLSL_PIXEL_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/FontPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/LinePixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/TriangleDepthPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/TrianglePixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/UIPixelShader.hlsl
			${PHYSICS_REPO_ROOT}/Assets/Shaders/DX/UIPixelShaderUntextured.hlsl
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
			${TEST_FRAMEWORK_ROOT}/Utils/AssetStream.cpp
			${TEST_FRAMEWORK_ROOT}/Utils/Log.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowLinux.cpp
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowLinux.h
		)
	endif()
		
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
		# macOS source files
		set(TEST_FRAMEWORK_SRC_FILES
			${TEST_FRAMEWORK_SRC_FILES}
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/FatalErrorIfFailedMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/FatalErrorIfFailedMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/PipelineStateMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/PipelineStateMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/PixelShaderMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RendererMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RendererMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RenderInstancesMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RenderInstancesMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RenderPrimitiveMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/RenderPrimitiveMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/TextureMTL.mm
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/TextureMTL.h
			${TEST_FRAMEWORK_ROOT}/Renderer/MTL/VertexShaderMTL.h
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/KeyboardMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/KeyboardMacOS.h
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/MouseMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Input/MacOS/MouseMacOS.h
			${TEST_FRAMEWORK_ROOT}/Utils/AssetStream.mm
			${TEST_FRAMEWORK_ROOT}/Utils/Log.mm
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowMacOS.mm
			${TEST_FRAMEWORK_ROOT}/Window/ApplicationWindowMacOS.h
		)

		# Metal shaders
		set(TEST_FRAMEWORK_SRC_FILES_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/VertexConstants.h
		)
		set(TEST_FRAMEWORK_METAL_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/FontShader.metal
			${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/LineShader.metal
			${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/TriangleShader.metal
			${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/UIShader.metal
		)

		# Compile Metal shaders
		foreach(SHADER ${TEST_FRAMEWORK_METAL_SHADERS})
			cmake_path(GET SHADER FILENAME AIR_SHADER)
			set(AIR_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${AIR_SHADER}.air")
			add_custom_command(OUTPUT ${AIR_SHADER}
				COMMAND xcrun -sdk macosx metal -c ${SHADER} -o ${AIR_SHADER}
				DEPENDS ${SHADER}
				COMMENT "Compiling ${SHADER}")
			list(APPEND TEST_FRAMEWORK_AIR_SHADERS ${AIR_SHADER})
		endforeach()

		# Link Metal shaders
		set(TEST_FRAMEWORK_METAL_LIB ${PHYSICS_REPO_ROOT}/Assets/Shaders/MTL/Shaders.metallib)
		add_custom_command(OUTPUT ${TEST_FRAMEWORK_METAL_LIB}
			COMMAND xcrun -sdk macosx metallib -o ${TEST_FRAMEWORK_METAL_LIB} ${TEST_FRAMEWORK_AIR_SHADERS}
			DEPENDS ${TEST_FRAMEWORK_AIR_SHADERS}
			COMMENT "Linking shaders")
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
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/VertexConstants.h
		)
		set(TEST_FRAMEWORK_GLSL_SHADERS
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/FontVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/LineVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/TriangleDepthVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/TriangleVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/UIVertexShader.vert
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/FontPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/LinePixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/TriangleDepthPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/TrianglePixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/UIPixelShader.frag
			${PHYSICS_REPO_ROOT}/Assets/Shaders/VK/UIPixelShaderUntextured.frag
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

	# Assets used by the test framework
	set(TEST_FRAMEWORK_ASSETS
		${PHYSICS_REPO_ROOT}/Assets/Fonts/Roboto-Regular.ttf
		${PHYSICS_REPO_ROOT}/Assets/UI.tga
		${TEST_FRAMEWORK_SRC_FILES_SHADERS}
		${TEST_FRAMEWORK_HLSL_VERTEX_SHADERS}
		${TEST_FRAMEWORK_HLSL_PIXEL_SHADERS}
		${TEST_FRAMEWORK_SPV_SHADERS}
		${TEST_FRAMEWORK_METAL_LIB}
	)

	# Group source files
	source_group(TREE ${TEST_FRAMEWORK_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES})

	# Group shader files
	source_group(TREE ${PHYSICS_REPO_ROOT} FILES ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_GLSL_SHADERS} ${TEST_FRAMEWORK_METAL_SHADERS})

	# Group intermediate files
	source_group(Intermediate FILES ${TEST_FRAMEWORK_SPV_SHADERS} ${TEST_FRAMEWORK_METAL_LIB})

	# Create TestFramework lib
	add_library(TestFramework STATIC ${TEST_FRAMEWORK_SRC_FILES} ${TEST_FRAMEWORK_SRC_FILES_SHADERS} ${TEST_FRAMEWORK_GLSL_SHADERS} ${TEST_FRAMEWORK_SPV_SHADERS} ${TEST_FRAMEWORK_METAL_SHADERS} ${TEST_FRAMEWORK_METAL_LIB})
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
	endif()
	if (LINUX)
		# Linux configuration
		target_link_libraries(TestFramework LINK_PUBLIC Jolt X11)
	endif()
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
		# macOS configuration
		target_link_libraries(TestFramework LINK_PUBLIC Jolt "-framework Cocoa -framework Metal -framework MetalKit -framework GameController")

		# Make sure that all test framework assets move to the Resources folder in the package
		foreach(ASSET_FILE ${TEST_FRAMEWORK_ASSETS})
			string(REPLACE ${PHYSICS_REPO_ROOT}/Assets "Resources" ASSET_DST ${ASSET_FILE})
			get_filename_component(ASSET_DST ${ASSET_DST} DIRECTORY)
			set_source_files_properties(${ASSET_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${ASSET_DST})
		endforeach()

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
