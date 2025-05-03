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
	${SAMPLES_ROOT}/Tests/Character/CharacterPlanetTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterPlanetTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterVirtualTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterVirtualTest.h
	${SAMPLES_ROOT}/Tests/Character/CharacterSpaceShipTest.cpp
	${SAMPLES_ROOT}/Tests/Character/CharacterSpaceShipTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConeConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConeConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintPriorityTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintPriorityTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintSingularityTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintSingularityTest.h
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintVsCOMChangeTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/ConstraintVsCOMChangeTest.h
	${SAMPLES_ROOT}/Tests/Constraints/DistanceConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/DistanceConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/FixedConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/FixedConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/GearConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/GearConstraintTest.h
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
	${SAMPLES_ROOT}/Tests/Constraints/PulleyConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/PulleyConstraintTest.h
	${SAMPLES_ROOT}/Tests/Constraints/RackAndPinionConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Constraints/RackAndPinionConstraintTest.h
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
	${SAMPLES_ROOT}/Tests/General/AllowedDOFsTest.cpp
	${SAMPLES_ROOT}/Tests/General/AllowedDOFsTest.h
	${SAMPLES_ROOT}/Tests/General/BigVsSmallTest.cpp
	${SAMPLES_ROOT}/Tests/General/BigVsSmallTest.h
	${SAMPLES_ROOT}/Tests/General/EnhancedInternalEdgeRemovalTest.cpp
	${SAMPLES_ROOT}/Tests/General/EnhancedInternalEdgeRemovalTest.h
	${SAMPLES_ROOT}/Tests/General/ShapeFilterTest.cpp
	${SAMPLES_ROOT}/Tests/General/ShapeFilterTest.h
	${SAMPLES_ROOT}/Tests/General/SimCollideBodyVsBodyTest.cpp
	${SAMPLES_ROOT}/Tests/General/SimCollideBodyVsBodyTest.h
	${SAMPLES_ROOT}/Tests/General/SimShapeFilterTest.cpp
	${SAMPLES_ROOT}/Tests/General/SimShapeFilterTest.h
	${SAMPLES_ROOT}/Tests/General/CenterOfMassTest.cpp
	${SAMPLES_ROOT}/Tests/General/CenterOfMassTest.h
	${SAMPLES_ROOT}/Tests/General/ChangeMotionQualityTest.cpp
	${SAMPLES_ROOT}/Tests/General/ChangeMotionQualityTest.h
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
	${SAMPLES_ROOT}/Tests/General/ConveyorBeltTest.cpp
	${SAMPLES_ROOT}/Tests/General/ConveyorBeltTest.h
	${SAMPLES_ROOT}/Tests/General/DampingTest.cpp
	${SAMPLES_ROOT}/Tests/General/DampingTest.h
	${SAMPLES_ROOT}/Tests/General/DynamicMeshTest.cpp
	${SAMPLES_ROOT}/Tests/General/DynamicMeshTest.h
	${SAMPLES_ROOT}/Tests/General/FrictionTest.cpp
	${SAMPLES_ROOT}/Tests/General/FrictionTest.h
	${SAMPLES_ROOT}/Tests/General/FrictionPerTriangleTest.cpp
	${SAMPLES_ROOT}/Tests/General/FrictionPerTriangleTest.h
	${SAMPLES_ROOT}/Tests/General/FunnelTest.cpp
	${SAMPLES_ROOT}/Tests/General/FunnelTest.h
	${SAMPLES_ROOT}/Tests/General/GravityFactorTest.cpp
	${SAMPLES_ROOT}/Tests/General/GravityFactorTest.h
	${SAMPLES_ROOT}/Tests/General/GyroscopicForceTest.cpp
	${SAMPLES_ROOT}/Tests/General/GyroscopicForceTest.h
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
	${SAMPLES_ROOT}/Tests/General/ModifyMassTest.cpp
	${SAMPLES_ROOT}/Tests/General/ModifyMassTest.h
	${SAMPLES_ROOT}/Tests/General/MultithreadedTest.cpp
	${SAMPLES_ROOT}/Tests/General/MultithreadedTest.h
	${SAMPLES_ROOT}/Tests/General/PyramidTest.cpp
	${SAMPLES_ROOT}/Tests/General/PyramidTest.h
	${SAMPLES_ROOT}/Tests/General/RestitutionTest.cpp
	${SAMPLES_ROOT}/Tests/General/RestitutionTest.h
	${SAMPLES_ROOT}/Tests/General/SensorTest.cpp
	${SAMPLES_ROOT}/Tests/General/SensorTest.h
	${SAMPLES_ROOT}/Tests/General/SimpleTest.cpp
	${SAMPLES_ROOT}/Tests/General/SimpleTest.h
	${SAMPLES_ROOT}/Tests/General/StackTest.cpp
	${SAMPLES_ROOT}/Tests/General/StackTest.h
	${SAMPLES_ROOT}/Tests/General/TwoDFunnelTest.cpp
	${SAMPLES_ROOT}/Tests/General/TwoDFunnelTest.h
	${SAMPLES_ROOT}/Tests/General/WallTest.cpp
	${SAMPLES_ROOT}/Tests/General/WallTest.h
	${SAMPLES_ROOT}/Tests/General/ActivateDuringUpdateTest.cpp
	${SAMPLES_ROOT}/Tests/General/ActivateDuringUpdateTest.h
	${SAMPLES_ROOT}/Tests/Rig/CreateRigTest.cpp
	${SAMPLES_ROOT}/Tests/Rig/CreateRigTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyBendConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyBendConstraintTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyContactListenerTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyContactListenerTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyCosseratRodConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyCosseratRodConstraintTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyCustomUpdateTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyCustomUpdateTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyForceTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyForceTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyFrictionTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyFrictionTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyGravityFactorTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyGravityFactorTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyKinematicTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyKinematicTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyLRAConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyLRAConstraintTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyPressureTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyPressureTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyRestitutionTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyRestitutionTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyShapesTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyShapesTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodySkinnedConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodySkinnedConstraintTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodySensorTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodySensorTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyStressTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyStressTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyUpdatePositionTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyUpdatePositionTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyVertexRadiusTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyVertexRadiusTest.h
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyVsFastMovingTest.cpp
	${SAMPLES_ROOT}/Tests/SoftBody/SoftBodyVsFastMovingTest.h
	${SAMPLES_ROOT}/Tests/Test.cpp
	${SAMPLES_ROOT}/Tests/Test.h
	${SAMPLES_ROOT}/Tests/Tools/LoadSnapshotTest.cpp
	${SAMPLES_ROOT}/Tests/Tools/LoadSnapshotTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/DynamicScaledShape.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/DynamicScaledShape.h
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
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledPlaneShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledPlaneShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledSphereShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledSphereShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCylinderShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTaperedCylinderShapeTest.h
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTriangleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/ScaledShapes/ScaledTriangleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/BoxShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/BoxShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/CapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/CapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/DeformedHeightFieldShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/DeformedHeightFieldShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/EmptyShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/EmptyShapeTest.h
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
	${SAMPLES_ROOT}/Tests/Shapes/MeshShapeUserDataTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/MeshShapeUserDataTest.h
	${SAMPLES_ROOT}/Tests/Shapes/SphereShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/SphereShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/PlaneShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/PlaneShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/RotatedTranslatedShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/RotatedTranslatedShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCapsuleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCapsuleShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCylinderShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/TaperedCylinderShapeTest.h
	${SAMPLES_ROOT}/Tests/Shapes/TriangleShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Shapes/TriangleShapeTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/MotorcycleTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/MotorcycleTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/TankTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/TankTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleConstraintTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleConstraintTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleSixDOFTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleSixDOFTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleStressTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleStressTest.h
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleTest.cpp
	${SAMPLES_ROOT}/Tests/Vehicle/VehicleTest.h
	${SAMPLES_ROOT}/Tests/Water/BoatTest.cpp
	${SAMPLES_ROOT}/Tests/Water/BoatTest.h
	${SAMPLES_ROOT}/Tests/Water/WaterShapeTest.cpp
	${SAMPLES_ROOT}/Tests/Water/WaterShapeTest.h
	${SAMPLES_ROOT}/Utils/ContactListenerImpl.cpp
	${SAMPLES_ROOT}/Utils/ContactListenerImpl.h
	${SAMPLES_ROOT}/Utils/DebugRendererSP.h
	${SAMPLES_ROOT}/Utils/RagdollLoader.cpp
	${SAMPLES_ROOT}/Utils/RagdollLoader.h
	${SAMPLES_ROOT}/Utils/ShapeCreator.cpp
	${SAMPLES_ROOT}/Utils/ShapeCreator.h
	${SAMPLES_ROOT}/Utils/SoftBodyCreator.cpp
	${SAMPLES_ROOT}/Utils/SoftBodyCreator.h
)

if (ENABLE_OBJECT_STREAM)
	set(SAMPLES_SRC_FILES
		${SAMPLES_SRC_FILES}
		${SAMPLES_ROOT}/Tests/Rig/BigWorldTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/BigWorldTest.h
		${SAMPLES_ROOT}/Tests/Rig/KinematicRigTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/KinematicRigTest.h
		${SAMPLES_ROOT}/Tests/Rig/LoadSaveBinaryRigTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/LoadSaveBinaryRigTest.h
		${SAMPLES_ROOT}/Tests/Rig/LoadSaveRigTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/LoadSaveRigTest.h
		${SAMPLES_ROOT}/Tests/Rig/LoadRigTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/LoadRigTest.h
		${SAMPLES_ROOT}/Tests/Rig/PoweredRigTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/PoweredRigTest.h
		${SAMPLES_ROOT}/Tests/Rig/RigPileTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/RigPileTest.h
		${SAMPLES_ROOT}/Tests/Rig/SkeletonMapperTest.cpp
		${SAMPLES_ROOT}/Tests/Rig/SkeletonMapperTest.h
	)
endif()

# Assets used by the samples
set(SAMPLES_ASSETS
	${PHYSICS_REPO_ROOT}/Assets/convex_hulls.bin
	${PHYSICS_REPO_ROOT}/Assets/heightfield1.bin
	${PHYSICS_REPO_ROOT}/Assets/Human/dead_pose1.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/dead_pose2.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/dead_pose3.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/dead_pose4.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/jog_hd.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/neutral.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/neutral_hd.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/skeleton_hd.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/sprint.tof
	${PHYSICS_REPO_ROOT}/Assets/Human/walk.tof
	${PHYSICS_REPO_ROOT}/Assets/Human.tof
	${PHYSICS_REPO_ROOT}/Assets/Racetracks/Zandvoort.csv
	${PHYSICS_REPO_ROOT}/Assets/terrain1.bof
	${PHYSICS_REPO_ROOT}/Assets/terrain2.bof
)

# Group source files
source_group(TREE ${SAMPLES_ROOT} FILES ${SAMPLES_SRC_FILES})

# Create Samples executable
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	# Icon
	set(JPH_ICON "${CMAKE_CURRENT_SOURCE_DIR}/macOS/icon.icns")
	set_source_files_properties(${JPH_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

	# macOS configuration
	add_executable(Samples MACOSX_BUNDLE ${SAMPLES_SRC_FILES} ${TEST_FRAMEWORK_ASSETS} ${SAMPLES_ASSETS} ${JPH_ICON})

	# Make sure that all samples assets move to the Resources folder in the package
	foreach(ASSET_FILE ${SAMPLES_ASSETS})
		string(REPLACE ${PHYSICS_REPO_ROOT}/Assets "Resources" ASSET_DST ${ASSET_FILE})
		get_filename_component(ASSET_DST ${ASSET_DST} DIRECTORY)
		set_source_files_properties(${ASSET_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${ASSET_DST})
	endforeach()

	set_property(TARGET Samples PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/iOS/SamplesInfo.plist")
	set_property(TARGET Samples PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.joltphysics.samples")
	set_property(TARGET Samples PROPERTY BUILD_RPATH "/usr/local/lib" INSTALL_RPATH "/usr/local/lib") # to find the Vulkan shared lib
else()
	add_executable(Samples ${SAMPLES_SRC_FILES})
endif()
target_include_directories(Samples PUBLIC ${SAMPLES_ROOT})
target_link_libraries(Samples LINK_PUBLIC TestFramework)

# Set the correct working directory
set_property(TARGET Samples PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")

# Make this project the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Samples")

