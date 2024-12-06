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
      [ "Multithreaded Access", "index.html#autotoc_md86", null ],
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
        [ "Vehicles", "md__docs_2_samples.html#autotoc_md5", null ],
        [ "Rig (Ragdolls)", "md__docs_2_samples.html#autotoc_md8", null ],
        [ "Soft Body", "md__docs_2_samples.html#autotoc_md9", null ],
        [ "Character", "md__docs_2_samples.html#autotoc_md11", null ],
        [ "Water", "md__docs_2_samples.html#autotoc_md12", null ],
        [ "Constraints", "md__docs_2_samples.html#autotoc_md13", null ],
        [ "General", "md__docs_2_samples.html#autotoc_md14", null ],
        [ "Shapes & Scaled Shapes", "md__docs_2_samples.html#autotoc_md15", null ]
      ] ]
    ] ],
    [ "Performance Test", "md__docs_2_performance_test.html", [
      [ "Commandline options", "md__docs_2_performance_test.html#autotoc_md2", null ],
      [ "Output", "md__docs_2_performance_test.html#autotoc_md6", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md7", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md17", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md18", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md19", null ]
      ] ],
      [ "v5.2.0", "md__docs_2_release_notes.html#autotoc_md20", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md21", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md22", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md23", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md24", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md25", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md26", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md27", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md29", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md31", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md33", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md35", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md37", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md38", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md40", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md42", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md43", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md45", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md46", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md47", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md48", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md49", null ]
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
      [ "Changes between v5.2.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md28", null ],
      [ "Changes between v5.1.0 and v5.2.0", "md__docs_2_a_p_i_changes.html#autotoc_md30", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md32", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md34", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md36", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md39", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md41", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md44", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md50", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md51", null ]
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
        [ "DirectX Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md83", null ],
        [ "Illegal Instruction Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md84", null ]
      ] ],
      [ "Doxygen on Windows", "md__build_2_r_e_a_d_m_e.html#autotoc_md85", null ]
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
"class_collide_sphere_vs_triangles.html#a1ba538db910e96e6bbeee5f29c7518ed",
"class_contact_constraint_manager_1_1_contact_allocator.html#a1f988bcee137591463666032293d8357",
"class_d_mat44.html#a896557b839b1a9126819a44a092f6bb6",
"class_double3.html#a7cdca7980db4588511c4851369dd9f2c",
"class_group_filter_table.html#a9a654e5e8db25dc647563a10a47d3ab4",
"class_ignore_single_body_filter_chained.html#ae841943960b45091f0fb253f086c6a12",
"class_mat44.html#a3aa8910fb3ed0cba6463518fe7d52328",
"class_mutable_compound_shape.html#a48172d558ec67cb67e4a4d0ec7b48133",
"class_path_constraint.html#a4a217906d4d990d4222d2dfbd3e3de5b",
"class_point_constraint.html#ab49bd368d3d5042348299f4bf76dd6d7",
"class_ragdoll.html#a9c9a6d0791b776f4360d0cd9ddfc6f84",
"class_scaled_shape.html#a1d1237076dc542497e7ca6b917cc7d68",
"class_skeletal_animation.html#a97437c1bae4626bb0c1e289db73dd712",
"class_soft_body_motion_properties.html#aec0e4f9a14bad868339c95357a8c37ab",
"class_static_array.html#a519467de065745ac462b4bb3878ef8a3",
"class_tapered_cylinder_shape.html#a147aacad19ced7e278abd34f6fcf2dd8",
"class_triangle_splitter.html#ad58e7ea8a806e7e9df8ea5117ff2ebe8",
"class_vector.html#a13077d539d6cac0b3415c96c54256a09",
"class_wheel.html#ad8cea661025b8c99b3fbd07f67880ebb",
"globals_s.html",
"struct_body_manager_1_1_draw_settings.html#a45faca582ce3ba4844789bccf736a965",
"struct_height_field_shape_1_1_h_s_get_triangles_context.html#a949f66e2b77e7cce9aff85ae630dfc74",
"struct_soft_body_shared_settings_1_1_face.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';