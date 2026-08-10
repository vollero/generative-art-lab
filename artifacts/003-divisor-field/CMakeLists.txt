add_executable(divisor_field src/main.cpp)
target_compile_features(divisor_field PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(divisor_field PRIVATE /W4 /permissive-)
else()
  target_compile_options(divisor_field PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(NAME artifact_003_smoke
    COMMAND divisor_field --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-003.svg
      --count 100 --columns 10 --width 640 --height 640)
  set_tests_properties(artifact_003_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 100 integers; maximum divisor count 12 at 60")

  add_test(NAME artifact_003_offset
    COMMAND divisor_field --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-003-offset.svg
      --start 11 --count 10 --columns 5)
  set_tests_properties(artifact_003_offset PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 10 integers; maximum divisor count 6 at 12")

  add_test(NAME artifact_003_rejects_zero_columns
    COMMAND divisor_field --columns 0)
  set_tests_properties(artifact_003_rejects_zero_columns PROPERTIES WILL_FAIL TRUE)

  add_test(NAME artifact_003_labeled_construction
    COMMAND divisor_field --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-003-labeled.svg
      --count 100 --columns 10 --show-values --show-records)
  set_tests_properties(artifact_003_labeled_construction PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 100 integers; maximum divisor count 12 at 60")
endif()
