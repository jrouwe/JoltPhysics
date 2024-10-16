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
      [ "Multithreaded Access", "index.html#autotoc_md82", null ],
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
      [ "General Controls", "md__docs_2_samples.html#autotoc_md5", null ],
      [ "The Tests", "md__docs_2_samples.html#autotoc_md6", [
        [ "Vehicles", "md__docs_2_samples.html#autotoc_md7", null ],
        [ "Rig (Ragdolls)", "md__docs_2_samples.html#autotoc_md8", null ],
        [ "Soft Body", "md__docs_2_samples.html#autotoc_md9", null ],
        [ "Character", "md__docs_2_samples.html#autotoc_md12", null ],
        [ "Water", "md__docs_2_samples.html#autotoc_md15", null ],
        [ "Constraints", "md__docs_2_samples.html#autotoc_md18", null ],
        [ "General", "md__docs_2_samples.html#autotoc_md25", null ],
        [ "Shapes & Scaled Shapes", "md__docs_2_samples.html#autotoc_md35", null ]
      ] ]
    ] ],
    [ "Performance Test", "md__docs_2_performance_test.html", [
      [ "Commandline options", "md__docs_2_performance_test.html#autotoc_md2", null ],
      [ "Output", "md__docs_2_performance_test.html#autotoc_md3", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md4", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md13", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md14", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md16", null ]
      ] ],
      [ "v5.1.0", "md__docs_2_release_notes.html#autotoc_md17", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md19", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md20", null ],
          [ "HeightField Shape", "md__docs_2_release_notes.html#autotoc_md21", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md22", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md23", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md24", null ]
        ] ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md26", null ]
      ] ],
      [ "v5.0.0", "md__docs_2_release_notes.html#autotoc_md27", [
        [ "New Functionality", "md__docs_2_release_notes.html#autotoc_md28", [
          [ "Soft Body", "md__docs_2_release_notes.html#autotoc_md29", null ],
          [ "Vehicles", "md__docs_2_release_notes.html#autotoc_md30", null ],
          [ "Character", "md__docs_2_release_notes.html#autotoc_md32", null ],
          [ "Constraints", "md__docs_2_release_notes.html#autotoc_md33", null ],
          [ "Collision Detection", "md__docs_2_release_notes.html#autotoc_md34", null ],
          [ "Simulation", "md__docs_2_release_notes.html#autotoc_md36", null ],
          [ "Various", "md__docs_2_release_notes.html#autotoc_md37", null ]
        ] ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md38", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md39", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md40", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md41", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md42", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md43", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md44", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md45", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md46", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md47", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md49", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md50", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md51", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md53", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md54", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md55", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md57", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md59", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md60", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md61", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md62", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v5.1.0 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md31", null ],
      [ "Changes between v5.0.0 and v5.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md48", null ],
      [ "Changes between v4.0.2 and v5.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md52", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md56", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md58", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md63", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md64", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md65", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md66", null ]
    ] ],
    [ "Building and Using Jolt Physics", "md__build_2_r_e_a_d_m_e.html", [
      [ "Build Types", "md__build_2_r_e_a_d_m_e.html#autotoc_md68", null ],
      [ "Includes", "md__build_2_r_e_a_d_m_e.html#autotoc_md69", null ],
      [ "Defines", "md__build_2_r_e_a_d_m_e.html#autotoc_md70", null ],
      [ "Logging & Asserting", "md__build_2_r_e_a_d_m_e.html#autotoc_md71", null ],
      [ "Custom Memory Allocator", "md__build_2_r_e_a_d_m_e.html#autotoc_md72", null ],
      [ "Building", "md__build_2_r_e_a_d_m_e.html#autotoc_md73", null ],
      [ "Other Build Tools", "md__build_2_r_e_a_d_m_e.html#autotoc_md74", null ],
      [ "Errors", "md__build_2_r_e_a_d_m_e.html#autotoc_md75", [
        [ "Link Error: File Format Not Recognized", "md__build_2_r_e_a_d_m_e.html#autotoc_md76", null ],
        [ "Link Error: Unresolved External Symbol", "md__build_2_r_e_a_d_m_e.html#autotoc_md77", null ],
        [ "DirectX Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md78", null ],
        [ "Illegal Instruction Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md79", null ]
      ] ],
      [ "Doxygen on Windows", "md__build_2_r_e_a_d_m_e.html#autotoc_md80", null ]
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
"_constraint_8h.html#adfc8350888df0c3a51714b7f372baf2dacd2c8bc6e5f1ea17c918ccaf89660104",
"_height_field_shape_8h.html#a1e58ce4b74ca86c27a71f785d9a2ec35",
"_object_stream_text_in_8cpp.html",
"_scope_exit_8h.html#a839a77790fe3ad876d339661b0dedc82",
"_sub_shape_i_d_8h_source.html",
"class_a_a_box.html#a8431a72347bfdb1a77ae45dcbba9c35b",
"class_body_creation_settings.html#a66c8700dfa9204a90b9bd7c7db50e85f",
"class_box_shape.html#a287e16289c491cd9dbd980e59ca7725b",
"class_character_base.html#a700e706b9c1710409fc16c6f9a083e0b",
"class_collision_group.html#a156fd729674403783be12fe41ee5b288",
"class_convex_hull_builder.html#aa49a23c340decb519ae98cfa2a9e469aa04477b2749feb6999bd3cb1c9f6fd963",
"class_d_vec3.html#a5563e51b263bc4c08fcfbc2babd3a7a5",
"class_e_p_a_convex_hull_builder.html#ae1b6e6eb12d8723eeab4265f9510b7c2",
"class_height_field_shape.html#acef4504a65c6f299d457b072dbe3789b",
"class_job_system_1_1_job_handle.html#a787e04af05eea5f3eda568956cca9f10",
"class_mesh_shape.html#a9996003623615ba708c420e90e86349e",
"class_object_stream_binary_in.html#ad454f1413f0c7a081d6de616336fda86",
"class_physics_scene_1_1_connected_constraint.html#abc1ce879afcaa745d1a73d4e6a793f84",
"class_quad_tree.html#a3f04fc0ab8ec76c39e7e4ac4679e6ffa",
"class_ref_const.html#acc0653f6d7ae2556d1a97230e87ccf2a",
"class_shape.html#a78cea5d82e89748412a5dca2f1352de1",
"class_slider_constraint.html#a3a2b09378bb8bf731279c89ab15152b0",
"class_soft_body_update_context.html#a6173bf5aad150857d14094419ea93db1",
"class_sub_shape_i_d.html#a93de7475867625a7b2dc053d2af57c2b",
"class_transformed_shape.html#a33b12d09ebaf2071dd49a3873978ac0a",
"class_vec3.html#a2f10743d69960eaac721399e4ede190a",
"class_vehicle_controller.html#a0a005727385292dd06ad30398e1eef0a",
"dir_a0f33af92addde396fc27b064c5eb8a9.html",
"md__docs_2_release_notes.html#autotoc_md44",
"struct_compound_shape_1_1_collide_compound_vs_shape_visitor.html#a80a7872b09611c352c0c02cf18a0fa67",
"struct_physics_update_context_1_1_step.html#af8fbffcba6d3b5adbdeb98c1d63903fe"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';