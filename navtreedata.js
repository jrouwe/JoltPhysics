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
      [ "Multithreaded Access", "index.html#autotoc_md97", null ],
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
    [ "Space Simulations", "index.html#space-simulations", null ],
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
      [ "Results", "md__docs_2_performance_test.html#autotoc_md5", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased Changes", "md__docs_2_release_notes.html#autotoc_md37", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md38", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md39", null ]
      ] ],
      [ "v5.3.0", "md__docs_2_release_notes.html#autotoc_md41", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md42", [
          [ "Samples", "md__docs_2_release_notes.html#autotoc_md43", null ],
          [ "MeshShape", "md__docs_2_release_notes.html#autotoc_md44", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md45", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md46", null ],
          [ "New Platforms", "md__docs_2_release_notes.html#autotoc_md49", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md50", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md51", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md56", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md57", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md58", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md59", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md60", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md61", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md62", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md63", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md64", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md65", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md66", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md67", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md68", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md69", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md70", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md71", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md72", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md73", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md74", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md75", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md76", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md77", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md78", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md79", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md80", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md81", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md82", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md83", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md84", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md85", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md86", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md87", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md88", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md89", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md90", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md91", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md92", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md93", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md94", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md95", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md96", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.3.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md33", null ],
      [ "Changes between v5.2.0 and v5.3.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md35", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md40", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md47", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md48", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md52", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md53", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md54", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md55", null ]
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
"_object_stream_8h.html#a8f946429439a23c334be4a1a80471ce1",
"_s_t_l_temp_allocator_8h.html",
"_stream_utils_8h.html#aab7d93bea0dbbfa623af7112db1bd8be",
"class_a_a_box.html#a376b8319c2ac3e843b1dfac1b8288e94",
"class_body.html#a6589465cd947d75f90b3975e6bdfd96e",
"class_body_lock_interface_no_lock.html#a49bd93a0cda1adce88e20921774d94a6",
"class_capsule_shape_1_1_capsule_no_convex.html",
"class_closest_hit_per_body_collision_collector.html#a077bbf531089d08686e8b7f9978ae035",
"class_constraint.html#a4b32ce7efd330ecf878f8ace4e0dd151",
"class_convex_shape_1_1_c_s_get_triangles_context.html#ac64202470dce73526cf31cae7fab85ba",
"class_debug_renderer_recorder.html#ab209b97db9eb9c3d5ee0735f7e1b4b3ea6e08a23df5a6495af4cfb4d55f9d83c4",
"class_float2.html#abb998c59ebedddb6fe8742b7e7f8e990",
"class_hinge_constraint_settings.html#a4069dacb8fa8c749be023b0817592b3c",
"class_large_island_splitter_1_1_splits.html#a0f8293e7c474ba25adfc13487fc005cca5f10b6f1afc0952ac2cda6048d1ef685",
"class_motion_properties.html#a9bf9425aab99ea920b73577819e579b1",
"class_object_stream_text_in.html#af69f711f9957bacae5aebfdac9d60b56",
"class_physics_system.html#ae7f5894935da5a4652a6d7ac60ad82e0",
"class_quat.html#adb1d1a70ed64c0a6fadc849126306706",
"class_rotated_translated_shape.html#a896a8828b726e57a15b696e85ce7f1c7",
"class_shape_filter.html#a34e0af56ee8eff11096ed8926e87f2c5",
"class_slider_constraint.html#af4b5d7b41aa1514342586fef277fabd5",
"class_soft_body_update_context.html#aeef08cf407990a9950c06a5418bd11e5ae16652901cd4816adf8c10e8468fb172",
"class_sub_shape_i_d_pair.html#a990a2912033187dda623e9ed0c265f9b",
"class_transformed_shape.html#a9f0559c4106b9b17becd5f401389b354",
"class_vec3.html#a6c6409cad56e4e357f73d43cd719ffe8",
"class_vehicle_controller_settings.html#a463f96865e70f8ebecaa95b4e71a4c4d",
"functions_d.html",
"md__docs_2_release_notes.html#autotoc_md57",
"struct_compound_shape_1_1_cast_shape_visitor.html#ae3d1ce73b6255f42f50e9acb10608477",
"struct_physics_update_context_1_1_step.html#a1f089199fc6b1d0fc6fb973d2750d32a"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';