add_executable(coprime_lattice src/main.cpp)
target_compile_features(coprime_lattice PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(coprime_lattice PRIVATE /W4 /permissive-)
else()
  target_compile_options(coprime_lattice PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(NAME artifact_004_extent_one
    COMMAND coprime_lattice --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-004-one.svg --extent 1)
  set_tests_properties(artifact_004_extent_one PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 8 lattice points; visible 8")

  add_test(NAME artifact_004_extent_ten
    COMMAND coprime_lattice --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-004-ten.svg --extent 10)
  set_tests_properties(artifact_004_extent_ten PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 440 lattice points; visible 256")

  add_test(NAME artifact_004_rejects_zero_extent COMMAND coprime_lattice --extent 0)
  set_tests_properties(artifact_004_rejects_zero_extent PROPERTIES WILL_FAIL TRUE)
endif()
