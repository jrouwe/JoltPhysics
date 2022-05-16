# Root
set(SAMPLES_ROOT ${PHYSICS_REPO_ROOT}/Samples)

# Source files
set(SAMPLES_SRC_FILES
	${SAMPLES_ROOT}/Layers.h
	${SAMPLES_ROOT}/Samples.cmake
	${SAMPLES_ROOT}/SamplesApp.cpp
	${SAMPLES_ROOT}/SamplesApp.h
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseCastRayTest.cpp
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseCastRayTest.h
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseInsertionTest.cpp
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseInsertionTest.h
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseTest.cpp
	${SAMPLES_ROOT}/Tests/BroadPhase/BroadPhaseTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterBaseTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterBaseTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterVirtualTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterVirtualTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConeConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConeConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintSingularityTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintSingularityTest.h
	${SAMPLES_ROOT}/Tests/Constraints/DistanceConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/DistanceConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/FixedConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/FixedConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/HingeConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/HingeConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/PointConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PointConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/PathConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PathConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/PoweredHingeConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PoweredHingeConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/PoweredSwingTwistConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PoweredSwingTwistConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/PoweredSliderConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PoweredSliderConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/SwingTwistConstraintFrictionTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/SwingTwistConstraintFrictionTest.h
	${SAMPLES_ROOT}/Tests/Constraints/SwingTwistConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/SwingTwistConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/SixDOFConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/SixDOFConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/SliderConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/SliderConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/SpringTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/SpringTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/CapsuleVsBoxTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/CapsuleVsBoxTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/ClosestPointTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/ClosestPointTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/ConvexHullTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/ConvexHullTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/ConvexHullShrinkTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/ConvexHullShrinkTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/EPATest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/EPATest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/InteractivePairsTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/InteractivePairsTest.h
	${SAMPLES_ROOT}/Tests/ConvexCollision/RandomRayTest.cpp
	${SAMPLES_ROOT}/Tests/ConvexCollision/RandomRayTest.h
	${SAMPLES_ROOT}/Tests/General/ActiveEdgesTest.cpp
	${SAMPLES_ROOT}/Tests/General/ActiveEdgesTest.h
	${SAMPLES_ROOT}/Tests/General/BigVsSmallTest.cpp
	${SAMPLES_ROOT}/Tests/General/BigVsSmallTest.h
	${SAMPLES_ROOT}/Tests/General/CenterOfMassTest.cpp
	${SAMPLES_ROOT}/Tests/General/CenterOfMassTest.h
	${SAMPLES_ROOT}/Tests/General/ChangeMotionTypeTest.cpp
	${SAMPLES_ROOT}/Tests/General/ChangeMotionTypeTest.h
	${SAMPLES_ROOT}/Tests/General/ChangeShapeTest.cpp
	${SAMPLES_ROOT}/Tests/General/ChangeShapeTest.h
	${SAMPLES_ROOT}/Tests/General/ChangeObjectLayerTest.cpp
	${SAMPLES_ROOT}/Tests/General/ChangeObjectLayerTest.h
	${SAMPLES_ROOT}/Tests/General/ContactListenerTest.cpp
	${SAMPLES_ROOT}/Tests/General/ContactListenerTest.h
	${SAMPLES_ROOT}/Tests/General/ContactManifoldTest.cpp
	${SAMPLES_ROOT}/Tests/General/ContactManifoldTest.h
	${SAMPLES_ROOT}/Tests/General/DampingTest.cpp
	${SAMPLES_ROOT}/Tests/General/DampingTest.h
	${SAMPLES_ROOT}/Tests/General/FrictionTest.cpp
	${SAMPLES_ROOT}/Tests/General/FrictionTest.h
	${SAMPLES_ROOT}/Tests/General/FrictionPerTriangleTest.cpp
	${SAMPLES_ROOT}/Tests/General/FrictionPerTriangleTest.h
	${SAMPLES_ROOT}/Tests/General/FunnelTest.cpp
	${SAMPLES_ROOT}/Tests/General/FunnelTest.h
	${SAMPLES_ROOT}/Tests/General/GravityFactorTest.cpp
	${SAMPLES_ROOT}/Tests/General/GravityFactorTest.h
	${SAMPLES_ROOT}/Tests/General/HeavyOnLightTest.cpp
	${SAMPLES_ROOT}/Tests/General/HeavyOnLightTest.h
	${SAMPLES_ROOT}/Tests/General/HighSpeedTest.cpp
	${SAMPLES_ROOT}/Tests/General/HighSpeedTest.h
	${SAMPLES_ROOT}/Tests/General/IslandTest.cpp
	${SAMPLES_ROOT}/Tests/General/IslandTest.h
	${SAMPLES_ROOT}/Tests/General/KinematicTest.cpp
	${SAMPLES_ROOT}/Tests/General/KinematicTest.h
	${SAMPLES_ROOT}/Tests/General/LoadSaveBinaryTest.cpp
	${SAMPLES_ROOT}/Tests/General/LoadSaveBinaryTest.h
	${SAMPLES_ROOT}/Tests/General/LoadSaveSceneTest.cpp
	${SAMPLES_ROOT}/Tests/General/LoadSaveSceneTest.h
	${SAMPLES_ROOT}/Tests/General/ManifoldReductionTest.cpp
	${SAMPLES_ROOT}/Tests/General/ManifoldReductionTest.h
	${SAMPLES_ROOT}/Tests/General/MultithreadedTest.cpp
	${SAMPLES_ROOT}/Tests/General/MultithreadedTest.h
	${SAMPLES_ROOT}/Tests/General/RestitutionTest.cpp
	${SAMPLES_ROOT}/Tests/General/RestitutionTest.h
	${SAMPLES_ROOT}/Tests/General/SensorTest.cpp
	${SAMPLES_ROOT}/Tests/General/SensorTest.h
	${SAMPLES_ROOT}/Tests/General/SimpleTest.cpp
	${SAMPLES_ROOT}/Tests/General/SimpleTest.h
	${SAMPLES_ROOT}/Tests/General/StackTest.cpp
	${SAMPLES_ROOT}/Tests/General/StackTest.h
	${SAMPLES_ROOT}/Tests/General/WallTest.cpp
	${SAMPLES_ROOT}/Tests/General/WallTest.h
	${SAMPLES_ROOT}/Tests/General/ActivateDuringUpdateTest.cpp
	${SAMPLES_ROOT}/Tests/General/ActivateDuringUpdateTest.h
	${SAMPLES_ROOT}/Tests/Rig/CreateRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/CreateRigTest.h
	${SAMPLES_ROOT}/Tests/Rig/KinematicRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/KinematicRigTest.h
	${SAMPLES_ROOT}/Tests/Rig/LoadSaveBinaryRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/LoadSaveBinaryRigTest.h
	${SAMPLES_ROOT}/Tests/Rig/LoadRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/LoadRigTest.h
	${SAMPLES_ROOT}/Tests/Rig/PoweredRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/PoweredRigTest.h
	${SAMPLES_ROOT}/Tests/Rig/RigPileTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/RigPileTest.h
	${SAMPLES_ROOT}/Tests/Test.cpp
	${SAMPLES_ROOT}/Tests/Test.h
	${SAMPLES_ROOT}/Tests/Tools/LoadSnapshotTest.cpp
	${SAMPLES_ROOT}/Tests/Tools/LoadSnapshotTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledBoxShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledBoxShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledCapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledCapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledStaticCompoundShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledStaticCompoundShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledMutableCompoundShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledMutableCompoundShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledOffsetCenterOfMassShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledOffsetCenterOfMassShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledConvexHullShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledConvexHullShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledCylinderShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledCylinderShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledHeightFieldShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledHeightFieldShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledMeshShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledMeshShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledSphereShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledSphereShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTriangleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTriangleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/BoxShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/BoxShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/CapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/CapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/StaticCompoundShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/StaticCompoundShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/MutableCompoundShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/MutableCompoundShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/OffsetCenterOfMassShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/OffsetCenterOfMassShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/ConvexHullShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/ConvexHullShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/CylinderShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/CylinderShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/HeightFieldShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/HeightFieldShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/MeshShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/MeshShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/SphereShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/SphereShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/RotatedTranslatedShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/RotatedTranslatedShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/TriangleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/TriangleShapeTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/TankTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/TankTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleConstraintTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleSixDOFTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleSixDOFTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleTest.h
	${SAMPLES_ROOT}/Tests/Water/WaterShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Water/WaterShapeTest.h
	${SAMPLES_ROOT}/Utils/ContactListenerImpl.cpp
	${SAMPLES_ROOT}/Utils/ContactListenerImpl.h
	${SAMPLES_ROOT}/Utils/RagdollLoader.cpp
	${SAMPLES_ROOT}/Utils/RagdollLoader.h
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# Enable Precompiled Headers for Samples
	set_source_files_properties(${SAMPLES_SRC_FILES} PROPERTIES COMPILE_FLAGS "/YuTestFramework.h")
	set(SAMPLES_SRC_FILES ${SAMPLES_SRC_FILES} ${SAMPLES_ROOT}/pch.cpp)
	set_source_files_properties(${SAMPLES_ROOT}/pch.cpp PROPERTIES COMPILE_FLAGS "/YcTestFramework.h")
endif()

# Group source files
source_group(TREE ${SAMPLES_ROOT} FILES ${SAMPLES_SRC_FILES})	

# Create Samples executable
add_executable(Samples  ${SAMPLES_SRC_FILES})
target_include_directories(Samples PUBLIC ${SAMPLES_ROOT})
target_link_libraries (Samples LINK_PUBLIC TestFramework d3d12.lib shcore.lib)

# Set the correct working directory
set_property(TARGET Samples PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")

# Make this project the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Samples")

