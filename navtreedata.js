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
      [ "Unreleased Changes", "md__docs_2_release_notes.html#autotoc_md45", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md46", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md47", null ]
      ] ],
      [ "v5.4.0", "md__docs_2_release_notes.html#autotoc_md48", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md49", null ],
        [ "Bug Fixes", "md__docs_2_release_notes.html#autotoc_md50", null ]
      ] ],
      [ "v5.3.0", "md__docs_2_release_notes.html#autotoc_md51", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md52", [
          [ "Samples", "md__docs_2_release_notes.html#autotoc_md53", null ],
          [ "MeshShape", "md__docs_2_release_notes.html#autotoc_md54", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md55", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md56", null ],
          [ "New Platforms", "md__docs_2_release_notes.html#autotoc_md57", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md58", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md59", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md60", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md61", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md62", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md63", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md64", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md65", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md66", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md67", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md68", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md69", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md70", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md71", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md72", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md73", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md74", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md75", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md76", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md77", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md78", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md79", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md80", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md81", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md82", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md83", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md84", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md85", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md86", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md87", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md88", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md89", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md90", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md91", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md92", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md93", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md94", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md95", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md96", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md97", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md98", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md99", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md100", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.4.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md33", null ],
      [ "Changes between v5.3.0 and v5.4.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v5.2.0 and v5.3.0", "md__docs_2_a_p_i_changes.html#autotoc_md35", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md37", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md38", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md40", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md41", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md42", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md43", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md44", null ]
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
"_half_float_8h_source.html",
"_object_stream_8h.html#a8731cf8e985c6fe5cb4e1dabf4206db2",
"_s_t_l_local_allocator_8h_source.html",
"_stream_utils_8h.html#a8e2ba0b1630f3d270dfdd4284a468ab9",
"class_a_a_b_b_tree_to_buffer.html#ae2034a353c9336f3e7df4bcdf9f97d8c",
"class_body.html#a5ffba4cea6db6696fff3db87999bdf42",
"class_body_lock_interface.html#ae7ab56617d45d42ee2935b0eaf9b57e6",
"class_capsule_shape.html#ac8ed07dee809bbe362b3dd4ec286aa35",
"class_character_vs_character_collision.html#a1336885e4964801d0812b3089faa674b",
"class_cone_constraint_settings.html#a18a734d20a0d8d9fbaf697523d43a53d",
"class_convex_shape.html#a6398de9c5aec64886a5184efb4fcca8f",
"class_debug_renderer_1_1_triangle.html#a6cf43c4aef393778a403a3f5659e3a97",
"class_fixed_constraint_settings.html#ae6bfde77eec2f9421c1af7670900bb70",
"class_hinge_constraint.html#a9695f177868118de9520091ac2eb8585",
"class_l_f_h_m_allocator.html",
"class_motion_properties.html#a4e7820bc189da3db3f2135686bfa0e9d",
"class_object_stream_in.html#ad45c7ba064bf63506d6d53d295b67d77",
"class_physics_system.html#a2f81c29dfb88dd7e0711caffcd2ebc22",
"class_quat.html#a60f30c650e027a892e699c156c7eca67",
"class_result.html#acc2f80e7cd0157e8428e1039bea3be9c",
"class_shape.html#a95d3060318d33c1ef314c5b856a70cb8",
"class_slider_constraint.html#a10e5c1d92324b6df087498f75b4f83e4",
"class_soft_body_shared_settings_1_1_skin_weight.html#a53c08815fb24f6bbf43a534b3cdcd61b",
"class_sub_shape_i_d_creator.html#a54023c5ee7e3547f2211e7a5ae9d620b",
"class_transformed_shape.html#a6634ea434c5877c1f458137d0b05e137",
"class_vec3.html#a3d4dc6f6807ead86b5150e7904ac80f5",
"class_vehicle_constraint_settings.html#a0d879ac8a47fb2584e23c6d0c5254d53",
"dir_166975991df1b4ecc6dd5a6639d45e50.html",
"md__docs_2_a_p_i_changes.html#autotoc_md36",
"struct_compound_shape_1_1_cast_ray_visitor.html#a848fb221cd472001c314498be3ad4f71",
"struct_physics_settings.html#a9aff2f46a311da4e48974dce7e3b4cee"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';