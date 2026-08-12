add_executable(fourier_epicycles src/main.cpp)
target_compile_features(fourier_epicycles PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(fourier_epicycles PRIVATE /W4 /permissive-)
else()
  target_compile_options(fourier_epicycles PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(NAME artifact_005_self_test COMMAND fourier_epicycles --self-test)
  set_tests_properties(artifact_005_self_test PROPERTIES
    PASS_REGULAR_EXPRESSION "Self-test passed")

  add_test(NAME artifact_005_smoke COMMAND fourier_epicycles
    --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-005.svg --samples 64 --terms 8)
  set_tests_properties(artifact_005_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 8 of 64 Fourier terms")

  add_test(NAME artifact_005_rejects_too_few_samples COMMAND fourier_epicycles --samples 3)
  set_tests_properties(artifact_005_rejects_too_few_samples PROPERTIES WILL_FAIL TRUE)
endif()
