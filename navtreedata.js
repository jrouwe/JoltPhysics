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
      [ "Mass Properties", "index.html#mass-properties", null ],
      [ "Degrees of Freedom", "index.html#degrees-of-freedom", null ],
      [ "Friction and Restitution", "index.html#friction-and-restitution", null ],
      [ "Linear and Angular Damping", "index.html#linear-and-angular-damping", null ],
      [ "Multithreaded Access", "index.html#multi-threaded-access", null ],
      [ "Single Threaded Access", "index.html#single-threaded-access", null ],
      [ "Shapes", "index.html#shapes", [
        [ "Sub Shape IDs", "index.html#sub-shape-ids", null ],
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
      [ "Constraint Motors", "index.html#constraint-motors", [
        [ "Stiffness and Damping", "index.html#stiffness-and-damping", null ],
        [ "Mass Normalized Stiffness and Damping", "index.html#mass-normalized-stiffness-and-damping", null ],
        [ "Frequency and Damping", "index.html#frequency-and-damping", null ]
      ] ],
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
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md46", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md47", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md48", null ],
        [ "Deprecated", "md__docs_2_release_notes.html#autotoc_md49", null ]
      ] ],
      [ "v5.5.0", "md__docs_2_release_notes.html#autotoc_md50", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md51", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md52", null ]
      ] ],
      [ "v5.4.0", "md__docs_2_release_notes.html#autotoc_md53", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md54", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md55", null ]
      ] ],
      [ "v5.3.0", "md__docs_2_release_notes.html#autotoc_md56", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md57", [
          [ "Samples", "md__docs_2_release_notes.html#autotoc_md58", null ],
          [ "MeshShape", "md__docs_2_release_notes.html#autotoc_md59", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md60", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md61", null ],
          [ "New Platforms", "md__docs_2_release_notes.html#autotoc_md62", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md63", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md64", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md65", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md66", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md67", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md68", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md69", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md70", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md71", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md72", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md73", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md74", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md75", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md76", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md77", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md78", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md79", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md80", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md81", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md82", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md83", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md84", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md85", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md86", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md87", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md88", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md89", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md90", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md91", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md92", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md93", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md94", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md95", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md96", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md97", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md98", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md99", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md100", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md101", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md102", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md103", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md104", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md105", null ]
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
"_compute_buffer_v_k_8h_source.html",
"_e_physics_update_error_8h.html",
"_hair_structs_8h_source.html",
"_mutable_compound_shape_8h.html",
"_ray_capsule_8h.html#a98c52ef6615d1b0452962366c52afc9d",
"_shape_8h.html#aebaf36faa66f5b466411acbe12342d70",
"_type_declarations_8h.html#a06065058d89eb12bed3964edef601a05",
"class_array.html#a981fa99850b2f859c24c554596a19cfb",
"class_body_creation_settings.html#aca747cc540a625d75c5a5abf0ad68216",
"class_box_shape.html#a5d9e2eb38ff5d85f18f838fbf3e69499",
"class_character_base.html#ab28f7afaa654cf8461f3638dd01334e0",
"class_collide_soft_body_vertex_iterator.html#a5d21cd50d1766e1d18daff58f58afbec",
"class_constraint.html#a632a8161ad49c443c2e3a8b600a730a1",
"class_convex_shape.html#a4cd6e2f87f30d108faf1f119dd73a06ba7a1920d61156abc05a60135aefe8bc67",
"class_debug_renderer_1_1_l_o_d.html",
"class_fixed_constraint_settings.html#a68029fead89e2efc72ddec38977086cf",
"class_hair_settings_1_1_gradient.html#a53864bf0836f066e330ae137e154dec0",
"class_i_object_stream_out.html",
"class_lock_free_hash_map.html#ab0b1ca4bac3c00813a024ed4b462e44d",
"class_motor_settings.html#a7dbbf359da6a861e6876f702650dd5ea",
"class_object_stream_text_out.html#acc5aff4eec5365eaf3b43070cd893485",
"class_plane.html#a71ec3665a225eda8f201e30fa2ee5544",
"class_r_t_t_i.html#a91cf4ad4740ffd16fe63e945ef288315",
"class_rotation_euler_constraint_part.html#ab5ff022582eb45db942a196a32795661",
"class_sim_shape_filter.html#ad8668ae5f0da7b59c8c0d37d0af8fce8",
"class_soft_body_contact_listener.html#a88fe3a41025942d5e4c1f144b20bfbd9",
"class_sphere_shape.html#a6a7e1d71ee2a8f08aa3fe20e079c586e",
"class_swing_twist_constraint.html#ab8a66fa8ace006007551e20f26859e60",
"class_triangle_codec_indexed8_bit_pack_s_o_a4_flags.html#adf7b4eade64871b3f53059bdacf9aacea61adc7a5c7a546e8f686916fe0f0029a",
"class_vec3.html#aeeadce7b9b442cccab2d2f3cbde2ccb9",
"class_vehicle_engine.html#a542f9131ecce2c3f41526c44759b913b",
"functions_func_f.html",
"md__docs_2_release_notes.html#autotoc_md49",
"struct_body_manager_1_1_draw_settings.html#a55d3594fa72dc4a59c2cffb9828c31f7",
"struct_h_l_s_l_to_c_p_p_1_1float2.html#aee9e3d0accf96e69062392a1d6fa4caf",
"struct_h_l_s_l_to_c_p_p_1_1uint3.html#a917f695479660d8854158c2bc8b01e4a",
"struct_physics_settings.html#a0bd7d6eb98c547488f874a9577fff451"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';