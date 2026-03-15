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
      [ "Multithreaded Access", "index.html#multi-threaded-access", null ],
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
      [ "Results", "md__docs_2_performance_test.html#autotoc_md4", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md35", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md36", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md41", null ]
      ] ],
      [ "v5.5.0", "md__docs_2_release_notes.html#autotoc_md45", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md46", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md51", null ]
      ] ],
      [ "v5.4.0", "md__docs_2_release_notes.html#autotoc_md52", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md53", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md54", null ]
      ] ],
      [ "v5.3.0", "md__docs_2_release_notes.html#autotoc_md55", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md56", [
          [ "Samples", "md__docs_2_release_notes.html#autotoc_md57", null ],
          [ "MeshShape", "md__docs_2_release_notes.html#autotoc_md58", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md59", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md60", null ],
          [ "New Platforms", "md__docs_2_release_notes.html#autotoc_md61", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md62", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md63", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md64", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md65", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md66", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md67", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md68", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md69", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md70", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md71", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md72", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md73", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md74", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md75", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md76", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md77", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md78", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md79", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md80", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md81", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md82", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md83", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md84", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md85", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md86", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md87", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md88", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md89", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md90", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md91", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md92", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md93", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md94", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md95", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md96", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md97", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md98", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md99", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md100", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md101", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md103", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md104", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.5.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md33", null ],
      [ "Changes between v5.4.0 and v5.5.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v5.3.0 and v5.4.0", "md__docs_2_a_p_i_changes.html#autotoc_md37", null ],
      [ "Changes between v5.2.0 and v5.3.0", "md__docs_2_a_p_i_changes.html#autotoc_md38", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md40", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md42", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md43", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md44", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md47", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md48", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md49", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md50", null ]
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
"_compute_queue_8h.html#a231baadb560d9857ca1536ad01a03cb0",
"_ellipse_8h.html",
"_hair_update_velocity_integrate_bindings_8h_source.html",
"_object_layer_8h.html#a744c7588b7b9c9d897854299a8682c66",
"_real_8h.html#a9829b84fb712d3deea004ccabed714a6",
"_skeleton_8cpp.html#a66c2401b3f742ef958235469e069214f",
"_type_declarations_8h.html#afe2e6efd40a082563570a021ccbb1f6f",
"class_array.html#af7a4d07255a8cfd0b90d171c12427d0d",
"class_body_interface.html",
"class_broad_phase.html#ad7d7ba03cddb7946ad487176458ffce4",
"class_character_contact_listener.html#a8aaba674bebace7120443c4e0bdfe2a1",
"class_collision_collector.html#ada5357990f959132a05d4d4d21f8c7b3",
"class_contact_constraint_manager.html#a3ede12b173e9c63f9421176573661b27",
"class_d_mat44.html#a14f62e1b746001ac1dac9af1ba432ad2",
"class_distance_constraint.html#a6783513d7658a4979f309be36a94f2d3",
"class_gear_constraint_part.html#ad3fc3cde9a29f5bbbba7db738f5715a3",
"class_hash_table_1_1iterator.html",
"class_island_builder.html#a213561deca43cb4c9bde6560bed5a0e1",
"class_mat44.html#aefb86266992d843e5de8e53cdbc5bdc6",
"class_mutex_array.html#a7440b62f6b4d1a8bfd608f88a26299e8",
"class_path_constraint.html#ae10bed0a11582fbe738f8339aa1257d5",
"class_polyhedron_submerged_volume_calculator.html#afd8348d65dfed4f7eb40ee92eb74742a",
"class_ragdoll_settings.html#a5ac758eeeb02faf2d90dbf70c874fda5",
"class_scaled_shape.html#a1a005f193965690c37b87fc7365f962a",
"class_skeletal_animation.html#a364b26f699464be275b70b5cf9f4f76c",
"class_soft_body_motion_properties.html#ab64fe12fb4966612d3de96f79a3e6991",
"class_static_array.html#a83ed073e3ed2248da10cef0277beeb23",
"class_tapered_cylinder_shape.html#a59bf42adddb7139199282ee755299782",
"class_two_body_constraint.html#a180665fea57c98679a6a25a89057ed06",
"class_vector.html#a97ea23f0596e27b87424d91cd79e19da",
"class_wheel_settings.html#a3981fc1b27c64cadc8a8a5e0d0e66521",
"globals_func_n.html",
"namespace_h_l_s_l_to_c_p_p.html#a801a8f2a94393b77b4f79a6d440d2365",
"struct_compound_shape_1_1_collect_transformed_shapes_visitor.html#a55cc06cb9e9d00cc51e902e0b038d96e",
"struct_h_l_s_l_to_c_p_p_1_1float4.html#a978328614ddd9d72465809d3b54a34f0",
"struct_hair_1_1_leaf_shape.html#aa434b71b9b9c10ffb92e42c92121fe85",
"struct_shape_cast_t.html#aed62d5fadde28ac5dea4f8a44d3dddf2"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';