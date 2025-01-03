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
      [ "General Controls", "md__docs_2_samples.html#autotoc_md3", null ],
      [ "The Tests", "md__docs_2_samples.html#autotoc_md4", [
        [ "Vehicles", "md__docs_2_samples.html#autotoc_md6", null ],
        [ "Rig (Ragdolls)", "md__docs_2_samples.html#autotoc_md8", null ],
        [ "Soft Body", "md__docs_2_samples.html#autotoc_md9", null ],
        [ "Character", "md__docs_2_samples.html#autotoc_md10", null ],
        [ "Water", "md__docs_2_samples.html#autotoc_md11", null ],
        [ "Constraints", "md__docs_2_samples.html#autotoc_md12", null ],
        [ "General", "md__docs_2_samples.html#autotoc_md13", null ],
        [ "Shapes & Scaled Shapes", "md__docs_2_samples.html#autotoc_md15", null ]
      ] ]
    ] ],
    [ "Performance Test", "md__docs_2_performance_test.html", [
      [ "Commandline options", "md__docs_2_performance_test.html#autotoc_md2", null ],
      [ "Output", "md__docs_2_performance_test.html#autotoc_md5", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md7", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md17", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md18", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md19", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md20", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md22", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md24", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md27", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md28", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md30", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md32", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md33", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md35", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md37", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md40", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md41", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md42", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md43", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md44", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md45", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md46", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md47", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md48", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md49", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md50", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md51", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md52", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md53", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md54", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md55", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md56", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md57", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md58", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md59", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md60", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md61", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md62", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md63", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md64", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md65", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md66", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md67", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md68", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md69", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md70", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.2.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md21", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md23", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md25", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md26", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md29", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md31", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md38", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ]
    ] ],
    [ "Building and Using Jolt Physics", "md__build_2_r_e_a_d_m_e.html", [
      [ "Build Types", "md__build_2_r_e_a_d_m_e.html#autotoc_md73", null ],
      [ "Includes", "md__build_2_r_e_a_d_m_e.html#autotoc_md74", null ],
      [ "Defines", "md__build_2_r_e_a_d_m_e.html#autotoc_md75", null ],
      [ "Logging & Asserting", "md__build_2_r_e_a_d_m_e.html#autotoc_md76", null ],
      [ "Custom Memory Allocator", "md__build_2_r_e_a_d_m_e.html#autotoc_md77", null ],
      [ "Building", "md__build_2_r_e_a_d_m_e.html#autotoc_md78", null ],
      [ "Other Build Tools", "md__build_2_r_e_a_d_m_e.html#autotoc_md79", null ],
      [ "Errors", "md__build_2_r_e_a_d_m_e.html#autotoc_md80", [
        [ "Link Error: File Format Not Recognized", "md__build_2_r_e_a_d_m_e.html#autotoc_md81", null ],
        [ "Link Error: Unresolved External Symbol", "md__build_2_r_e_a_d_m_e.html#autotoc_md82", null ],
        [ "Link Error: Undefined Symbol", "md__build_2_r_e_a_d_m_e.html#autotoc_md83", null ],
        [ "DirectX Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md84", null ],
        [ "Illegal Instruction Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md85", null ]
      ] ],
      [ "Doxygen on Windows", "md__build_2_r_e_a_d_m_e.html#autotoc_md86", null ]
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
        [ "Typedefs", "functions_type.html", null ],
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
"_constraint_8h.html#adfc8350888df0c3a51714b7f372baf2da5ad55d96abf0e50647d6de116530d6df",
"_hash_combine_8h.html#a8ce241fe287672b4a559499d341d2ee7",
"_object_stream_binary_in_8h.html",
"_scale_helpers_8h.html#a7445e71cd125deb818c71e867ea9275e",
"_string_tools_8cpp.html#a479eac2df35b47887f8dbfa014f6923d",
"class_a_a_box.html#a0982651a7b0ed25cab78537cdeba982a",
"class_body.html#adb1bb49db094ac2ef7c4951d07ba8417",
"class_body_manager.html#a877463f0189e8e131edec5d83413726b",
"class_character.html#a17120469ace1ad77f381e4ab2223ad65",
"class_collide_sphere_vs_triangles.html#a0ceac76dc9e4731e8309af5fb46bb27c",
"class_contact_constraint_manager_1_1_contact_allocator.html#a1968130257ab4041d6e18c676869fdd3",
"class_d_mat44.html#a7d108f0330578e1274b9232c28a39326",
"class_double3.html#a6e5ecbdc4bec96b0a033ad090f0a38ba",
"class_group_filter_table.html#a6b25689191afbfdb272feb4e57df4035",
"class_ignore_single_body_filter_chained.html#a92bbf17cd704ae6e64c4d7c86bfcf7fc",
"class_mat44.html#a3445a728646fe8ad80381a0e55ea2289",
"class_mutable_compound_shape.html#a3992cb7a984307a08139812e44da14ae",
"class_path_constraint.html#a488ab67aba069c0fa0c81194afc74bd1",
"class_point_constraint.html#a988ec795ba7e071d1cd256afc5edd6cf",
"class_ragdoll.html#a86b347f11599361d0cf8bddcacad4295",
"class_s_t_l_temp_allocator.html#afe0ff9b69805be725b54c35f238f8c6f",
"class_skeletal_animation.html#a33027b83da1fc52f6cf8058559d90083",
"class_soft_body_motion_properties.html#ad7ecbec356136a9cb231fb0695a07720",
"class_static_array.html#a39b29025d859bb426416085c054dfb89",
"class_tapered_capsule_shape_settings.html#a9b510c75cb2e66ba52919ac871c69f8e",
"class_triangle_splitter.html#a29fdff34f24303ee9d3c52dec0ad73b3",
"class_vec4.html#aff8d5cf747d2cfa658ac6bc5822b6434",
"class_wheel.html#ad03e749022ce451237c8731a7b59eb79",
"globals_o.html",
"struct_body_manager_1_1_body_stats.html#af523de7c042e3938507160de0377356c",
"struct_height_field_shape_1_1_h_s_get_triangles_context.html#a4e6aec803eb1738b1f7c90f05100b422",
"struct_soft_body_shared_settings_1_1_edge.html#a2542e95a101443bb88ef41b38e569a8d"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';