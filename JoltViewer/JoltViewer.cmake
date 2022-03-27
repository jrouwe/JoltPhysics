# Root
set(JOLT_VIEWER_ROOT ${PHYSICS_REPO_ROOT}/JoltViewer)

# Source files
set(JOLT_VIEWER_SRC_FILES
	${JOLT_VIEWER_ROOT}/JoltViewer.cmake
	${JOLT_VIEWER_ROOT}/JoltViewer.cpp
	${JOLT_VIEWER_ROOT}/JoltViewer.h
)

# Group source files
source_group(TREE ${JOLT_VIEWER_ROOT} FILES ${JOLT_VIEWER_SRC_FILES})	

# Create JoltViewer executable
add_executable(JoltViewer ${JOLT_VIEWER_SRC_FILES})
target_include_directories(JoltViewer PUBLIC ${JOLT_VIEWER_ROOT})
target_link_libraries (JoltViewer LINK_PUBLIC TestFramework d3d12.lib shcore.lib)

# Set the correct working directory
set_property(TARGET JoltViewer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")
