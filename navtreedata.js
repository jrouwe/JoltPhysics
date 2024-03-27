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
      [ "Multithreaded Access", "index.html#autotoc_md61", null ],
      [ "Single Threaded Access", "index.html#single-threaded-access", null ],
      [ "Shapes", "index.html#shapes", [
        [ "Creating Shapes", "index.html#creating-shapes", null ],
        [ "Saving Shapes", "index.html#saving-shapes", null ],
        [ "Convex Radius", "index.html#convex-radius", null ],
        [ "Center of Mass", "index.html#center-of-mass", null ],
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
      [ "General Controls", "md__docs_2_samples.html#autotoc_md2", null ],
      [ "The Tests", "md__docs_2_samples.html#autotoc_md3", [
        [ "Vehicles", "md__docs_2_samples.html#autotoc_md4", null ],
        [ "Rig (Ragdolls)", "md__docs_2_samples.html#autotoc_md7", null ],
        [ "Soft Body", "md__docs_2_samples.html#autotoc_md10", null ],
        [ "Character", "md__docs_2_samples.html#autotoc_md11", null ],
        [ "Water", "md__docs_2_samples.html#autotoc_md12", null ],
        [ "Constraints", "md__docs_2_samples.html#autotoc_md13", null ],
        [ "General", "md__docs_2_samples.html#autotoc_md14", null ],
        [ "Shapes & Scaled Shapes", "md__docs_2_samples.html#autotoc_md21", null ]
      ] ]
    ] ],
    [ "Performance Test", "md__docs_2_performance_test.html", [
      [ "Commandline options", "md__docs_2_performance_test.html#autotoc_md5", null ],
      [ "Output", "md__docs_2_performance_test.html#autotoc_md6", null ],
      [ "Results", "md__docs_2_performance_test.html#autotoc_md8", null ]
    ] ],
    [ "Release Notes", "md__docs_2_release_notes.html", [
      [ "Unreleased changes", "md__docs_2_release_notes.html#autotoc_md15", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md16", null ],
        [ "Improvements", "md__docs_2_release_notes.html#autotoc_md17", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md19", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md20", null ]
      ] ],
      [ "v4.0.2", "md__docs_2_release_notes.html#autotoc_md22", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md23", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md24", null ]
      ] ],
      [ "v4.0.1", "md__docs_2_release_notes.html#autotoc_md25", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md26", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md27", null ]
      ] ],
      [ "v4.0.0", "md__docs_2_release_notes.html#autotoc_md28", [
        [ "New functionality", "md__docs_2_release_notes.html#autotoc_md29", null ],
        [ "Removed functionality", "md__docs_2_release_notes.html#autotoc_md30", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md31", null ],
        [ "Bug fixes", "md__docs_2_release_notes.html#autotoc_md32", null ]
      ] ],
      [ "v3.0.0", "md__docs_2_release_notes.html#autotoc_md33", null ],
      [ "v2.0.1", "md__docs_2_release_notes.html#autotoc_md34", null ],
      [ "v2.0.0", "md__docs_2_release_notes.html#autotoc_md35", [
        [ "Major new functionality", "md__docs_2_release_notes.html#autotoc_md36", null ],
        [ "New supported compilers", "md__docs_2_release_notes.html#autotoc_md37", null ],
        [ "New supported platforms", "md__docs_2_release_notes.html#autotoc_md38", null ]
      ] ],
      [ "v1.1.0", "md__docs_2_release_notes.html#autotoc_md39", null ],
      [ "v1.0.0", "md__docs_2_release_notes.html#autotoc_md40", null ]
    ] ],
    [ "Breaking API Changes", "md__docs_2_a_p_i_changes.html", [
      [ "Changes between v4.0.2 and latest", "md__docs_2_a_p_i_changes.html#autotoc_md41", null ],
      [ "Changes between v4.0.0 and v4.0.2", "md__docs_2_a_p_i_changes.html#autotoc_md42", null ],
      [ "Changes between v3.0.1 and v4.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md43", null ],
      [ "Changes between v2.0.1 and v3.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md44", null ],
      [ "Changes between v1.1.0 and v2.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md45", null ],
      [ "Changes between v1.0.0 and v1.1.0", "md__docs_2_a_p_i_changes.html#autotoc_md46", null ],
      [ "Changes between v0.0.0 and v1.0.0", "md__docs_2_a_p_i_changes.html#autotoc_md47", null ]
    ] ],
    [ "Building and Using Jolt Physics", "md__build_2_r_e_a_d_m_e.html", [
      [ "Build Types", "md__build_2_r_e_a_d_m_e.html#autotoc_md49", null ],
      [ "Includes", "md__build_2_r_e_a_d_m_e.html#autotoc_md50", null ],
      [ "Defines", "md__build_2_r_e_a_d_m_e.html#autotoc_md51", null ],
      [ "Logging & Asserting", "md__build_2_r_e_a_d_m_e.html#autotoc_md52", null ],
      [ "Custom Memory Allocator", "md__build_2_r_e_a_d_m_e.html#autotoc_md53", null ],
      [ "Building", "md__build_2_r_e_a_d_m_e.html#autotoc_md54", null ],
      [ "Other Build Tools", "md__build_2_r_e_a_d_m_e.html#autotoc_md55", null ],
      [ "Link Errors", "md__build_2_r_e_a_d_m_e.html#autotoc_md56", null ],
      [ "DirectX Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md57", null ],
      [ "Illegal Instruction Error", "md__build_2_r_e_a_d_m_e.html#autotoc_md58", null ],
      [ "Doxygen on Windows", "md__build_2_r_e_a_d_m_e.html#autotoc_md59", null ]
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
"_constraint_manager_8h.html#acbbaf4aa1ca19ae6a91452046a99062c",
"_indexed_triangle_8h.html#a36dddc469fdd08411ce6fb341a59f7bd",
"_path_constraint_8cpp.html",
"_serializable_attribute_typed_8h.html#a564e5bd12360a28431c73574690dd1b8",
"_triangle_grouper_closest_centroid_8cpp.html",
"class_angle_constraint_part.html#ab75081badd77c6854423f45825ff36ce",
"class_body_interface.html#a7669d5c31627d2c99fac69723fb1a219",
"class_broad_phase_layer_interface_table.html#accae5298efb3880af7dd16c8ba4a9e72",
"class_closest_hit_collision_collector.html#aa9679e95456d67c8f813e6f13de6e65c",
"class_constraint.html#af62388f6f234f8cd7d24af34e49180fd",
"class_cylinder_shape.html#a6685737b774a4fdbcc626aae25f27770",
"class_default_broad_phase_layer_filter.html#af08a7bc5141b24385c4fc1a731657cda",
"class_get_triangles_context_multi_vertex_list.html#aa9cd165a98d96373c2d112a9c8ec258f",
"class_job_system.html",
"class_matrix.html#a7b98e3c672cc83e50b0cb6cf33b1ec65",
"class_object_layer_pair_filter_table.html",
"class_physics_material.html#af8673b7c2cefec46b38fd90fb8fe7784",
"class_quad_tree.html#ab5538217d4a9a42986da28d64b65740d",
"class_ref_target_virtual.html#a716938dd06b8a5be71588e501c8c2b7d",
"class_shape_cast_settings.html#a3f2bb437820c80a4ba4e2952fe9742d0",
"class_slider_constraint_settings.html#a8f4226328c8052b155b772c932bcb244",
"class_sphere_shape.html#a67684ecf3156eed16a5da749fcaad002",
"class_swing_twist_constraint_settings.html#a0f7ae7835e07b547eae7141543f3bbd5",
"class_triangle_splitter_longest_axis.html#a6e0fd86ca2d38f41bd6faffcd94e08a4",
"class_vec8.html#a5b527ce885b1b22c56173d4e48425bb3",
"class_wheel.html#ab93b1087453249915b72e74733ea2af3",
"globals_g.html",
"struct_broad_phase_1_1_update_state.html",
"struct_minkowski_difference.html#a9ec95eca1b2f1e8420746f40e6544f85",
"struct_triangle_splitter_1_1_range.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';