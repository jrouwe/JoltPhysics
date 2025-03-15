/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Jolt Physics", "index.html", [
    [ "Architecture of Jolt Physics", "index.html#architecture-jolt-physics", null ],
    [ "Getting Started", "index.html#getting-started", null ],
    [ "Bodies", "index.html#bodies", [
      [ "Types", "index.html#body-types", null ],
      [ "Creating Bodies", "index.html#creating-bodies", null ],
      [ "Multithreaded Access", "index.html#autotoc_md87", null ],
      [ "Single Threaded Access", "index.html#single-threaded-access", null ],
      [ "Shapes", "index.html#shapes", [
        [ "Dynamic Mesh Shapes", "index.html#dynamic-mesh-shapes", null ],
        [ "Creating Shapes", "index.html#creating-shapes", null ],
        [ "Saving Shapes", "index.html#saving-shapes", null ],
        [ "Convex Radius", "index.html#convex-radius", null ],
        [ "Center of Mass", "index.html#center-of-mass", null ],
        [ "Scaling Shapes", "index.html#scaling-shapes", null ],
        [ "Creating Custom Shapes", "index.html#creating-custom-shapes", null ]
      ] ],
      [ "Sensors", "index.html#sensors", null ],
      [ "Sleeping", "index.html#sleeping-bodies", null ],
      [ "Soft Bodies", "index.html#soft-bodies", [
        [ "Soft Body Contact Listeners", "index.html#soft-body-contact-listener", null ],
        [ "Skinning Soft Bodies", "index.html#skinning-soft-bodies", null ],
        [ "Soft Body Work In Progress", "index.html#soft-body-wip", null ]
      ] ]
    ] ],
    [ "Constraints", "index.html#constraints", [
      [ "Constraint Motors", "index.html#constraint-motors", null ],
      [ "Breakable Constraints", "index.html#breakable-constraints", null ]
    ] ],
    [ "Collision Detection", "index.html#collision-detection", [
      [ "Broad Phase", "index.html#broad-phase", null ],
      [ "Narrow Phase", "index.html#narrow-phase", null ],
      [ "Collision Filtering", "index.html#collision-filtering", null ],
      [ "Level of Detail", "index.html#level-of-detail", null ],
      [ "Continuous Collision Detection", "index.html#continuous-collision-detection", null ],
      [ "Ghost Collisions", "index.html#ghost-collisions", null ]
    ] ],
    [ "Character Controllers", "index.html#character-controllers", null ],
    [ "The Simulation Step", "index.html#the-simulation-step", null ],
    [ "Conventions and Limits", "index.html#conventions-and-limits", null ],
    [ "Big Worlds", "index.html#big-worlds", null ],
    [ "Deterministic Simulation", "index.html#deterministic-simulation", null ],
    [ "Rolling Back a Simulation", "index.html#rolling-back-a-simulation", null ],
    [ "Being Sloppy While Still Being Deterministic", "index.html#sloppy-determinism", null ],
    [ "Working With Multiple Physics Systems", "index.html#working-with-multiple-physics-systems", null ],
    [ "Debug Rendering", "index.html#debug-rendering", null ],
    [ "Memory Management", "index.html#memory-management", null ],
    [ "The Simulation Step in Detail", "index.html#the-simulation-step-in-detail", [
      [ "Broad Phase Update Prepare", "index.html#broad-phase-update-prepare", null ],
      [ "Broad Phase Update Finalize", "index.html#broad-phase-update-finalize", null ],
      [ "Step Listeners", "index.html#step-listeners-update", null ],
      [ "Apply Gravity", "index.html#apply-gravity-update", null ],
      [ "Determine Active Constraints", "index.html#determine-active-constraints", null ],
      [ "Build Islands from Constraints", "index.html#build-islands-from-constraints", null ],
      [ "Find Collisions", "index.html#find-collisions", null ],
      [ "Setup Velocity Constraints", "index.html#setup-velocity-constraints", null ],
      [ "Finalize Islands", "index.html#finalize-islands", null ],
      [ "Set Body Island Idx", "index.html#set-body-island-idx", null ],
      [ "Solve Velocity Constraints", "index.html#solve-velocity-constraints", null ],
      [ "Pre Integrate", "index.html#pre-integrate", null ],
      [ "Integrate & Clamp Velocities", "index.html#integrate-and-clamp-velocities", null ],
      [ "Post Integrate", "index.html#post-integrate", null ],
      [ "Find CCD Contacts", "index.html#find-ccd-contacts", null ],
      [ "Resolve CCD Contacts", "index.html#resolve-ccd-contacts", null ],
      [ "Finalize Contact Cache, Contact Removed Callbacks", "index.html#finalize-contact-cache", null ],
      [ "Solve Position Constraints, Update Bodies Broad Phase", "index.html#solve-position-constraints", null ],
      [ "Soft Body Prepare", "index.html#soft-body-prepare", null ],
      [ "Soft Body Collide", "index.html#soft-body-collide", null ],
      [ "Soft Body Simulate", "index.html#soft-body-simulate", null ],
      [ "Soft Body Finalize", "index.html#soft-body-finalize", null ]
    ] ],
    [ "Jolt Physics Samples", "md__docs_2_samples.html", [
      [ "General Controls", "md__docs_2_samples.html#autotoc_md7", null ],
      [ "The Tests", "md__docs_2_samples.html#autotoc_md8", [
        [ "Vehicles", "md__docs_2_samples.html#autotoc_md9", null ],
        [ "Rig (Ragdolls)", "md__docs_2_samples.html#autotoc_md10", null ],
        [ "Soft Body", "md__docs_2_samples.html#autotoc_md11", null ],
        [ "Character", "md__docs_2_samples.html#autotoc_md12", null ],
        [ "Water", "md__docs_2_samples.html#autotoc_md13", null ],
        [ "Constraints", "md__docs_2_samples.html#autotoc_md14", null ],
        [ "General", "md__docs_2_samples.html#autotoc_md15", null ],
        [ "Shapes & Scaled Shapes", "md__docs_2_samples.html#autotoc_md16", null ]
      ] ]
    ] ],
    [ "Performance Test", "md__docs_2_performance_test.html", [
      [ "Commandline options", "md__docs_2_performance_test.html#autotoc_md2", null ],
      [ "Output", "md__docs_2_performance_test.html#autotoc_md3", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md4", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md43", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md44", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md45", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md46", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md47", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md48", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md49", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md50", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md51", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md52", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md53", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md54", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md55", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md56", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md57", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md58", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md59", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md60", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md61", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md62", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md63", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md64", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md65", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md66", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md67", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md68", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md69", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md70", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md71", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md72", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md73", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md74", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md75", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md76", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md77", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md78", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md79", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md80", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md81", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md82", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md83", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md84", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md85", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md86", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.2.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md33", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md35", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md37", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md38", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md40", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md41", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md42", null ]
    ] ],
    [ "Building and Using Jolt Physics", "md__build_2_r_e_a_d_m_e.html", [
      [ "Build Types", "md__build_2_r_e_a_d_m_e.html#autotoc_md18", null ],
      [ "Includes", "md__build_2_r_e_a_d_m_e.html#autotoc_md19", null ],
      [ "Defines", "md__build_2_r_e_a_d_m_e.html#autotoc_md20", null ],
      [ "Logging & Asserting", "md__build_2_r_e_a_d_m_e.html#autotoc_md21", null ],
      [ "Custom Memory Allocator", "md__build_2_r_e_a_d_m_e.html#autotoc_md22", null ],
      [ "Building", "md__build_2_r_e_a_d_m_e.html#autotoc_md23", null ],
      [ "Other Build Tools", "md__build_2_r_e_a_d_m_e.html#autotoc_md24", null ],
      [ "Errors", "md__build_2_r_e_a_d_m_e.html#autotoc_md25", [
        [ "Link Error: File Format Not Recognized", "md__build_2_r_e_a_d_m_e.html#autotoc_md26", null ],
        [ "Link Error: Unresolved External Symbol", "md__build_2_r_e_a_d_m_e.html#autotoc_md27", null ],
        [ "Link Error: Undefined Symbol", "md__build_2_r_e_a_d_m_e.html#autotoc_md28", null ],
        [ "DirectX Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md29", null ],
        [ "Illegal Instruction Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md30", null ]
      ] ],
      [ "Doxygen on Windows", "md__build_2_r_e_a_d_m_e.html#autotoc_md31", null ]
    ] ],
    [ "Projects Using Jolt", "md__docs_2_projects_using_jolt.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", "functions_type" ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"_a_a_b_b_tree_builder_8cpp.html",
"_constraint_8h.html#adfc8350888df0c3a51714b7f372baf2da18205404fe6507d42b3b2d25aaf62f21",
"_hash_combine_8h.html",
"_object_stream_8h.html#aad34e2f2bc03433e8db638ac1d14a0f0",
"_scale_helpers_8h.html",
"_stream_wrapper_8h.html",
"class_a_a_box.html#a5c69aa69cf37132aa62dc0b58571e79c",
"class_body.html#a7797f43ac6078b682b4c9cb6c7b9b18e",
"class_body_lock_interface_no_lock.html#a6ecbc0cbe52514464956d3a49b73d79f",
"class_capsule_shape_1_1_capsule_with_convex.html",
"class_closest_hit_per_body_collision_collector.html#a8fdc1795a765488d0380ebaff6a1c5ae",
"class_constraint.html#a56624d10dcf6dd18fa8903897fcc2a03",
"class_convex_shape_1_1_support.html#ab2a715898d419c03321366a009539ae0",
"class_debug_renderer_recorder.html#ae016c0c6e1951199b26be47d0e58e2e4",
"class_float3.html#a4d2277dc1efc95eb284c72c1313809fc",
"class_hinge_constraint_settings.html#a9c33cc4f73d2d92dac9b270e1e15a216",
"class_large_island_splitter_1_1_splits.html#a39339a4cb247742be9ae64489c2758e7",
"class_motion_properties.html#aad3b333b7af3e3b39be95edf9b69d133",
"class_object_stream_text_out.html#a22b16b26d56a883b07fac755cdc8aaeb",
"class_physics_system_1_1_bodies_to_sleep.html#afe53f6626002783f517ee62f61160ba8",
"class_quat.html#ae53457812ce9ffa5fb7b6b8a3c5c97e5",
"class_rotated_translated_shape.html#ab85aaba12f9b0080fb7eee3a49530efe",
"class_shape_functions.html#a054634bc3e0037afe94af128994d5261",
"class_slider_constraint_settings.html#a090c1e3dd8e6d8651d8287b4dbe5fc46",
"class_specified_broad_phase_layer_filter.html#aedde7395b8b32b17168accd40be9860d",
"class_swing_twist_constraint.html#a44ce1ccf1c4d95f88bb6ab2571035196",
"class_triangle.html#aafe91e923394d83ed4ee872bc2164ca5",
"class_vec3.html#abaa362706e12d5e3fa8d950ba80d173e",
"class_vehicle_engine.html#a542f9131ecce2c3f41526c44759b913b",
"functions_func_p.html",
"md__docs_2_release_notes.html#autotoc_md78",
"struct_compound_shape_1_1_collide_point_visitor.html#a462fecb2d7a11225bc4b89015a4c6e9b",
"struct_physics_update_context_1_1_step.html#ad8016f86c45bb3092259e91ff0f6c426"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';