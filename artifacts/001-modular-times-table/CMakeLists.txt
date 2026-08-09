add_executable(modular_times_table src/main.cpp)
target_compile_features(modular_times_table PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(modular_times_table PRIVATE /W4 /permissive-)
else()
  target_compile_options(modular_times_table PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(
    NAME artifact_001_smoke
    COMMAND modular_times_table
      --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-001-smoke.svg
      --points 120
      --multiplier 2.0
      --width 640
      --height 640
  )
  set_tests_properties(artifact_001_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 120 chords"
  )

  add_test(NAME artifact_001_rejects_too_few_points
    COMMAND modular_times_table --points 2)
  set_tests_properties(artifact_001_rejects_too_few_points PROPERTIES
    WILL_FAIL TRUE
  )

  add_test(
    NAME artifact_001_construction_layer
    COMMAND modular_times_table
      --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-001-construction.svg
      --points 20
      --multiplier 2
      --chords 7
      --highlight 6
      --show-points
      --show-labels
  )
  set_tests_properties(artifact_001_construction_layer PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 7 chords"
  )
endif()
