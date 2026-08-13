add_executable(modular_flow src/main.cpp)
target_compile_features(modular_flow PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(modular_flow PRIVATE /W4 /permissive-)
else()
  target_compile_options(modular_flow PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(NAME artifact_006_self_test COMMAND modular_flow --self-test)
  set_tests_properties(artifact_006_self_test PROPERTIES PASS_REGULAR_EXPRESSION "Self-test passed")

  add_test(NAME artifact_006_modulus_five COMMAND modular_flow
    --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-006-five.svg --modulus 5 --show-labels)
  set_tests_properties(artifact_006_modulus_five PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 5 residues; cycles 2; longest cycle 1; maximum depth 2")

  add_test(NAME artifact_006_rejects_small_modulus COMMAND modular_flow --modulus 1)
  set_tests_properties(artifact_006_rejects_small_modulus PROPERTIES WILL_FAIL TRUE)
endif()
