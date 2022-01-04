# Jolt Physics Samples

This document describes the demos in the Samples application (currently compiles only under Windows). When you run the samples application the application will initially start paused, press P to unpause it. The menu is accessible through pressing ESC, it has the following options:

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

### Rig (Ragdolls)

This category demonstrates how ragdolls can be made and controlled using keyframing or motors.

|[![Kinematic Ragdoll](https://img.youtube.com/vi/gvq6qdU3ZTs/hqdefault.jpg)](https://www.youtube.com/watch?v=gvq6qdU3ZTs)|
|:-|
|*A ragdoll set to kinematic mode (infinite mass, simulated using velocities only) interacting with dynamic objects.*|

|[![Ragdoll Driven to Animated Pose](https://img.youtube.com/vi/lYHhe6HLbs4/hqdefault.jpg)](https://www.youtube.com/watch?v=lYHhe6HLbs4)|
|:-|
|*Demonstrating a humanoid ragdoll driven by motors which are trying to match a sprint animation in local space (green sticks).*|

|[![160 Ragdolls in a Pile](https://img.youtube.com/vi/pwyCW0yNKMA/hqdefault.jpg)](https://www.youtube.com/watch?v=pwyCW0yNKMA)|
|:-|
|*160 Ragdolls being dropped on a scene from Horizon Zero Dawn.*|

|[![160 Ragdolls in a Pile (Sleeping Visualization)](https://img.youtube.com/vi/7ZMm7yObpqs/hqdefault.jpg)](https://www.youtube.com/watch?v=7ZMm7yObpqs)|
|:-|
|*160 Ragdolls dropping on a pile, simulated using the Jolt Physics engine. Yellow means the ragdoll is simulated, red means the simulation is sleeping.*|

|[![160 Ragdolls Driven to Pose](https://img.youtube.com/vi/jhpsIqbsU4I/hqdefault.jpg)](https://www.youtube.com/watch?v=jhpsIqbsU4I)|
|:-|
|*A pile of ragdolls that are driven to a specific animated death pose. This gives the ragdolls 'stiffness'.*|

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

|[![Swing Twist Constraing](https://img.youtube.com/vi/8aQ9x8SQSuM/hqdefault.jpg)](https://www.youtube.com/watch?v=8aQ9x8SQSuM)|
|:-|
|*Demonstrates a chain of swing-twist constraints (usable for humanoid shoulders). The green cones show the swing limit and the pink pie shows the twist limit.*|

### General

This category contains general simulation tests. It demonstrates things like friction, restitution, damping, modifying gravity and continous collision detection. Some highlights:

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

### Shapes & Scaled Shapes

These categories show off all of the supported shapes and how they can be scaled at run-time.

|[![Shape Scaling](https://img.youtube.com/vi/u9cPBGUFurc/hqdefault.jpg)](https://www.youtube.com/watch?v=u9cPBGUFurc)|
|:-|
|*A height field shape using various scales in Jolt Physics: Uniform, Non uniform, Mirrored, Inside out*|
