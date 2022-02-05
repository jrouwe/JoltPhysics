# Performance Test

The performance test application contains a couple of simple scenes to test performance of Jolt Physics. It will output the results to the TTY in CSV format.

## Commandline options

- -s=[scene]: This allows you to select a scene, [scene] can be;
    - Ragdoll: A scene with 16 piles of 10 ragdolls (3680 bodies) with motors active dropping on a level section.
    - ConvexVsMesh: A simpler scene of 484 convex shapes (sphere, box, convex hull, capsule) falling on a 2000 triangle mesh.
- -q=[quality]: This limits the motion quality types that the test will run on. By default it will test both. [quality] can be:
    - Discrete: Discrete collision detection
    - LinearCast: Linear cast continous collision detection
- -t=[num]: This sets the amount of threads the test will run on. By default it will test 1 .. number of virtual processors.
- -p: Outputs a profile snapshot every 100 iterations
- -r: Outputs a performance_test_[tag].jor file that contains a recording to be played back with JoltViewer
- -f: Outputs the time taken per frame to per_frame_[tag].csv
- -h: Displays a help text

## Output

- Motion Quality: Shows the motion quality for the test.
- Thread Count: The amount of threads used for the test.
- Steps / Second: Average amount of physics steps / second over the entire duration of the test.
- Hash: A hash of all positions and rotations of the bodies at the end of the test. Can be used to verify that the test was deterministic.

## Results

If you're interested in how Jolt scales with multiple CPUs and compares to other physics engines, take a look at [this document](https://jrouwe.nl/jolt/JoltPhysicsMulticoreScaling.pdf).
