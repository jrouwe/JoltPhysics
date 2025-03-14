# Jolt Physics Samples

This document describes the demos in the [Samples](https://github.com/jrouwe/JoltPhysics/tree/master/Samples) application. When you run the samples application the application will initially start paused, press P to unpause it. The menu is accessible through pressing ESC, it has the following options:

* Select Test - This allows you to select between the different types of physics tests
* Test Settings - Some tests will allow extra configuration, if not this setting will be greyed out
* Restart Test (R) - When selecting this, the test will go back to its initial state
* Run All Tests - This will run every tests for 10 seconds before proceeding to the next. This is a good way of visually inspecting the simulation before commiting a code change.
* Next Test (N) - When running all tests, this option can be used to quickly skip to the next test.
* Physics Settings - This menu contains all physics configuration.
* Drawing Options - This menu shows all the options for drawing the internal state of the physics simulation.
* Mouse Probe - This allows you to switch between various collision detection modes to test the different collision detection algorithms
* Shoot Object - A sample application is not complete without being able to shoot some balls at the simulation (B key). This menu allows additional settings.
* Help - A quick help text.

## General Controls

* Use the Mouse and WSAD keys to move around, hold Shift to speed up and Ctrl to slow down
* Hold the Space key to pick up an object in the center of the screen and move it around with the mouse and WSAD.
* P - Pause / unpause simulation.
* O - Single step the simulation.
* , - Step back (only when Physics Settings / Record State for Playback is on).
* . - Step forward (only when Physics Settings / Record State for Playback is on).
* Shift + , - Play reverse (only when Physics Settings / Record State for Playback is on).
* Shift + . - Replay forward (only when Physics Settings / Record State for Playback is on).
* T - Dump frame timing information to profile_*.html (when JPH_PROFILE_ENABLED defined).

## The Tests

Note that you can watch all movies below in [a single YouTube playlist](https://www.youtube.com/watch?v=pwyCW0yNKMA&list=PLYXVwtOr1CBxbA50jVg2dKUQvHW_5OOom).

### Vehicles

This categories shows vehicles created through the VehicleConstraint. These vehicles use ray- or shape casts to detect collision with the ground and simulate a vehicle with an engine, gearbox, differentials and suspension.

|[![Vehicle Demo](https://img.youtube.com/vi/A_gvLH4KKDA/hqdefault.jpg)](https://www.youtube.com/watch?v=A_gvLH4KKDA)|
|:-|
|*A wheeled vehicle.*|

|[![Tank Demo](https://img.youtube.com/vi/QwlPOKbxsqU/hqdefault.jpg)](https://www.youtube.com/watch?v=QwlPOKbxsqU)|
|:-|
|*Demonstrates a tracked vehicle with a turret constrained to the main body with hinge constraints.*|

|[![Motorcycle Demo](https://img.youtube.com/vi/umI8FF0gVxs/hqdefault.jpg)](https://www.youtube.com/watch?v=umI8FF0gVxs)|
|:-|
|*Demonstrates a motor cycle.*|

|[![Vehicle Gravity Override](https://img.youtube.com/vi/AJPS31S6ZO8/hqdefault.jpg)](https://www.youtube.com/watch?v=AJPS31S6ZO8)|
|:-|
|*Applying a custom gravity override to a vehicle to create weird gameplay.*|

### Rig (Ragdolls)

This category demonstrates how ragdolls can be made and controlled using keyframing or motors.

|[![Kinematic Ragdoll](https://img.youtube.com/vi/gvq6qdU3ZTs/hqdefault.jpg)](https://www.youtube.com/watch?v=gvq6qdU3ZTs)|
|:-|
|*A ragdoll set to kinematic mode (infinite mass, simulated using velocities only) interacting with dynamic objects.*|

|[![Ragdoll Driven to Animated Pose](https://img.youtube.com/vi/lYHhe6HLbs4/hqdefault.jpg)](https://www.youtube.com/watch?v=lYHhe6HLbs4)|
|:-|
|*Demonstrating a humanoid ragdoll driven by motors which are trying to match a sprint animation in local space (green sticks).*|

|[![Skeleton Mapper](https://img.youtube.com/vi/hrnmgNN-m-U/hqdefault.jpg)](https://www.youtube.com/watch?v=hrnmgNN-m-U)|
|:-|
|*An animation is played back on a high detail skeleton ('Animation') and then mapped onto a low detail ragdoll skeleton ('Reversed Mapped'). This animation is used to drive the motors of the ragdoll. The resulting pose is mapped back to the high detail skeleton ('Mapped'). Note that the skeletons are drawn offset to make them clearer..*|

|[![160 Ragdolls in a Pile](https://img.youtube.com/vi/pwyCW0yNKMA/hqdefault.jpg)](https://www.youtube.com/watch?v=pwyCW0yNKMA)|
|:-|
|*160 Ragdolls being dropped on a scene from Horizon Zero Dawn.*|

|[![160 Ragdolls in a Pile (Sleeping Visualization)](https://img.youtube.com/vi/7ZMm7yObpqs/hqdefault.jpg)](https://www.youtube.com/watch?v=7ZMm7yObpqs)|
|:-|
|*160 Ragdolls dropping on a pile, simulated using the Jolt Physics engine. Yellow means the ragdoll is simulated, red means the simulation is sleeping.*|

|[![160 Ragdolls Driven to Pose](https://img.youtube.com/vi/jhpsIqbsU4I/hqdefault.jpg)](https://www.youtube.com/watch?v=jhpsIqbsU4I)|
|:-|
|*A pile of ragdolls that are driven to a specific animated death pose. This gives the ragdolls 'stiffness'.*|

### Soft Body

|[![Soft Body Demo](https://img.youtube.com/vi/vJX_3FNISkw/hqdefault.jpg)](https://www.youtube.com/watch?v=vJX_3FNISkw)|
|:-|
|*Demonstrates Soft Body physics as simulated by Jolt Physics. Soft body physics can be used for things like cloth and soft balls.*|

|[![Soft Body Contact Listener Demo](https://img.youtube.com/vi/DmS_8d2bdOw/hqdefault.jpg)](https://www.youtube.com/watch?v=DmS_8d2bdOw)|
|:-|
|*Demonstrates the use of soft body contact listeners. You can use these to affect the collision response between a soft body and a rigid body by e.g. artificially making the mass of one of the two higher so that the other is less affected by the collision. Finally you can also turn a contact into a sensor contact which means you get the contact points but there will not be any collision response..*|

|[![Soft Body Bend Constraints Demo](https://img.youtube.com/vi/A1iswelnGH4/hqdefault.jpg)](https://www.youtube.com/watch?v=A1iswelnGH4)|
|:-|
|*This video shows the effect of bend constraints on a wrinkled cloth. The left most patch has no constraints to preserve the wrinkles, the middle uses distance constrains ('sticks') to preserve the wrinkles and the last one uses dihedral angle constraints to preserve the angle between two triangles on their shared edge.*|

|[![Soft Body Skin Constraints Demo](https://img.youtube.com/vi/NXw8yMczHJg/hqdefault.jpg)](https://www.youtube.com/watch?v=NXw8yMczHJg)|
|:-|
|*This demo shows a soft body that is connected to a skinned mesh via distance constraints. Each simulated vertex can deviate from its skinned position by a fixed length. The green lines indicate the animated joints of the skinned mesh.*|

### Character

This category shows how you can simulate a (humanoid) character using a capsule.

|[![Character Demo](https://img.youtube.com/vi/YjaJT9of7UE/hqdefault.jpg)](https://www.youtube.com/watch?v=YjaJT9of7UE)|
|:-|
|*A demonstration of a game Character. Demonstrates moving, sliding against the environment, crouching and jumping.*|

### Water

This category shows how you can implement a water simulation in your game.

|[![Water Simulation](https://img.youtube.com/vi/CEr_LtQLGeg/hqdefault.jpg)](https://www.youtube.com/watch?v=CEr_LtQLGeg)|
|:-|
|*Water buoyancy and friction simulation. Demonstrates how various shapes and compound shapes behave in the water. The right most object has a lowered center of mass.*|

### Constraints

This category shows the various constraints that are supported. Constraints connect two or more bodies together and limit the relative movement.

|[![Path Constraint](https://img.youtube.com/vi/6xMKNMjD5pE/hqdefault.jpg)](https://www.youtube.com/watch?v=6xMKNMjD5pE)|
|:-|
|*Showing the path constraint in action.*|

|[![Swing Twist Constraint](https://img.youtube.com/vi/8aQ9x8SQSuM/hqdefault.jpg)](https://www.youtube.com/watch?v=8aQ9x8SQSuM)|
|:-|
|*Demonstrates a chain of swing-twist constraints (usable for humanoid shoulders). The green cones show the swing limit and the pink pie shows the twist limit.*|

|[![Gear constraint](https://img.youtube.com/vi/3w5SgElroBw/hqdefault.jpg)](https://www.youtube.com/watch?v=3w5SgElroBw)|
|:-|
|*Demonstrates a gear constraint. Note that the gears can be placed at any relative angle of each other, so you could e.g. create a bevel or worm gear.*|

|[![Rack and pinion constraint](https://img.youtube.com/vi/e588KG-ZSxc/hqdefault.jpg)](https://www.youtube.com/watch?v=e588KG-ZSxc)|
|:-|
|*Demonstrates a rack and pinion constraint.*|

|[![Pulley constraint](https://img.youtube.com/vi/9P8OaahtU-4/hqdefault.jpg)](https://www.youtube.com/watch?v=9P8OaahtU-4)|
|:-|
|*Shows two boxes connected through a pulley constraint. In this case the constraint is configured as a block and tackle with and advantage of 2: the right block moves 2x as slow as the left block.*|

### General

This category contains general simulation tests. It demonstrates things like friction, restitution, damping, modifying gravity and continuous collision detection. Some highlights:

|[![Stable Box Stacking](https://img.youtube.com/vi/fTtjBLYBxco/hqdefault.jpg)](https://www.youtube.com/watch?v=fTtjBLYBxco)|
|:-|
|*A YouTube video showing stability of a pile of boxes.*|

|[![Active Edge Detection](https://img.youtube.com/vi/EanFxlkZgcA/hqdefault.jpg)](https://www.youtube.com/watch?v=EanFxlkZgcA)|
|:-|
|*Demonstrates objects sliding along a polygon mesh. Internal mesh edges are ignored and do not cause objects to bounce off.*|

|[![Funnel Test](https://img.youtube.com/vi/Y-UgylH992A/hqdefault.jpg)](https://www.youtube.com/watch?v=Y-UgylH992A)|
|:-|
|*1000 random shapes in a funnel.*|

|[![Multithreaded Island Simulation](https://img.youtube.com/vi/_Lv5xlWtCpM/hqdefault.jpg)](https://www.youtube.com/watch?v=_Lv5xlWtCpM)|
|:-|
|*We will automatically split up the simulation in islands of non-interacting bodies and distribute the work across multiple threads. Each island has its own color.*|

|[![Single vs Double Precision](https://img.youtube.com/vi/KGnlYSW3550/hqdefault.jpg)](https://www.youtube.com/watch?v=KGnlYSW3550)|
|:-|
|*Shows the difference between compiling Jolt Physics in single precision and double precision (define JPH_DOUBLE_PRECISION).*|

|[![Conveyor belt](https://img.youtube.com/vi/p_H6egZzbZE/hqdefault.jpg)](https://www.youtube.com/watch?v=p_H6egZzbZE)|
|:-|
|*A demo of setting the surface velocity of a body to create a conveyor belt. The boxes have decreasing friction from front to back (last one has zero friction so slowly slides down the ramp).*|

### Shapes & Scaled Shapes

These categories show off all of the supported shapes and how they can be scaled at run-time.

|[![Shape Scaling](https://img.youtube.com/vi/u9cPBGUFurc/hqdefault.jpg)](https://www.youtube.com/watch?v=u9cPBGUFurc)|
|:-|
|*A height field shape using various scales in Jolt Physics: Uniform, Non uniform, Mirrored, Inside out*|
