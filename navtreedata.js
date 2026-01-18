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
      [ "Output", "md__docs_2_performance_test.html#autotoc_md4", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md5", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md46", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md47", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md48", null ]
      ] ],
      [ "v5.5.0", "md__docs_2_release_notes.html#autotoc_md49", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md50", null ],
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
      [ "Changes between v5.3.0 and v5.4.0", "md__docs_2_a_p_i_changes.html#autotoc_md35", null ],
      [ "Changes between v5.2.0 and v5.3.0", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md37", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md38", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md40", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md41", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md42", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md43", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md44", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md45", null ]
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
"_empty_shape_8h.html",
"_half_float_8h.html",
"_object_stream_8cpp.html",
"_register_types_8h.html#a1e0db6031789e773039c7fc15ef47057",
"_soft_body_contact_listener_8h.html#a146313e7653a5364d1e51eccb325abaaa4c312fb94f5fa9ebabb3968b56ed0221",
"_vec3_8h_source.html",
"class_array_1_1crev__it.html#ab19a933566b5b46e9bfc5ca33db43fa4",
"class_body_interface.html#a1a67ebf158316c8ce40768aab624680b",
"class_broad_phase_brute_force.html#a7e8ac19b326adf19996bd1288089018b",
"class_character_i_d.html#ae3b2f60c91eb99486dc977adb67f5ad1",
"class_collision_group.html#aa7e60aa0b9f6e5563ab4ca690c8e2b56",
"class_contact_constraint_manager.html#ac438b9748b641a504658e700f0e43305",
"class_d_mat44.html#a4c37a399a449603c4e611b81d58e004d",
"class_distance_constraint_settings.html#a6ccc7f6a9630c0c6f818a82c37640943",
"class_get_triangles_context_vertex_list.html#ac42ac006966e5e9819b97f3764be4fc4",
"class_height_field_shape.html#a37e87278a6969e16bacab304c47a4a0f",
"class_job_system.html#a51c615924cf4f8eefb8e3fbe72041373",
"class_matrix.html#a82a128d3923926e678aa6acc94f2f05d",
"class_node_codec_quad_tree_half_float_1_1_encoding_context.html",
"class_path_constraint_path.html#aae9ea1aede3ece72c5620bf88776757d",
"class_profile_thread.html#a6d9ed54e26b6b8e37967a8dce6226e66",
"class_ray_cast_result.html",
"class_scaled_shape.html#aca3913c54c796de4014330c508484005",
"class_skeleton.html",
"class_soft_body_motion_properties.html#afcbcb8a86be08dddf2f33027741d321b",
"class_static_compound_shape.html#a15254cd1ef605397c34cd1455c3ce317",
"class_tapered_cylinder_shape_1_1_tapered_cylinder.html#a1a34aff6da2ce5e2a274d484a8460cb4",
"class_u_vec4.html#a1dad43e9c11de6562500394e3c07a491",
"class_vehicle_collision_tester.html#a168d396a1e2431a984b36e83c2affe07",
"class_wheel_settings_w_v.html#a352e4f9e4960e63fd95257d7649db932",
"globals_r.html",
"namespace_h_l_s_l_to_c_p_p.html#aaa9f23adc71fa1c3103e4a1f3ccc4d2b",
"struct_compound_shape_1_1_collide_compound_vs_shape_visitor.html#aadc26efc419ef5f50163fcbb07d7d404",
"struct_h_l_s_l_to_c_p_p_1_1float4.html#ae5d78466084c590fcefe21a09b4e264f",
"struct_hair_settings_1_1_material.html#a000e25a378fbc7c94e5a437b82f35e50",
"struct_soft_body_shared_settings_1_1_edge.html#a6330e512065ff11935b9fb0f04b5db6c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';