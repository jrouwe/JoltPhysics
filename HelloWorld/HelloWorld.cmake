# Root
set(HELLO_WORLD_ROOT ${PHYSICS_REPO_ROOT}/HelloWorld)

# Source files
set(HELLO_WORLD_SRC_FILES
	${HELLO_WORLD_ROOT}/HelloWorld.cpp
	${HELLO_WORLD_ROOT}/HelloWorld.cmake
)

# Group source files
source_group(TREE ${HELLO_WORLD_ROOT} FILES ${HELLO_WORLD_SRC_FILES})
