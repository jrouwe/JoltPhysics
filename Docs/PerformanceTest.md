# Performance Test

The performance test application contains a couple of simple scenes to test performance of Jolt Physics. It will output the results to the TTY in CSV format.

## Commandline options

- -s=[scene]: This allows you to select a scene, [scene] can be;
    - Ragdoll: A scene with 16 piles of 10 ragdolls (3680 bodies) with motors active dropping on a level section.
	- RagdollSinglePile: A single pile of 160 ragdolls (3680 bodies) with motors active dropping on a level section.
    - ConvexVsMesh: A simpler scene of 484 convex shapes (sphere, box, convex hull, capsule) falling on a 2000 triangle mesh.
	- Pyramid: A pyramid of 1240 boxes stacked on top of each other to profile large island splitting.
	- LargeMesh: Searches for the biggest MeshShape that can be created and then drops 4410 boxes on that mesh.
- -i=[iterations]: Number of physics steps before the test finishes.
- -q=[quality]: This limits the motion quality types that the test will run on. By default it will test both. [quality] can be:
    - Discrete: Discrete collision detection
    - LinearCast: Linear cast continous collision detection
- -t=[num]: This sets the amount of threads the test will run on. By default it will test 1 .. number of virtual processors. Can be 'max' to run on as many thread as the CPU has.
- -no_sleep: Disable sleeping.
- -p: Outputs a profile snapshot every 100 iterations
- -r: Outputs a performance_test_[tag].jor file that contains a recording to be played back with JoltViewer
- -f: Outputs the time taken per frame to per_frame_[tag].csv
- -h: Displays a help text
- -rs: Record the simulation state in state_[tag].bin.
- -vs: Validate the recorded simulation state from state_[tag].bin. This will after every simulation step check that the state is the same as the recorded state and trigger a breakpoint if this is not the case. This is used to validate cross platform determinism.
- -repeat=[num]: Repeats all tests num times.
- -validate_hash=[hash]: Will validate that the hash of the simulation matches the supplied hash. Program terminates with return code 1 if it doesn't. Can be used to automatically validate determinism.

## Output

- Motion Quality: Shows the motion quality for the test.
- Thread Count: The amount of threads used for the test.
- Steps / Second: Average amount of physics steps / second over the entire duration of the test.
- Hash: A hash of all positions and rotations of the bodies at the end of the test. Can be used to verify that the test was deterministic.

## Results

If you're interested in how Jolt scales with multiple CPUs and compares to other physics engines, take a look at [this document](https://jrouwe.nl/jolt/JoltPhysicsMulticoreScaling.pdf).
