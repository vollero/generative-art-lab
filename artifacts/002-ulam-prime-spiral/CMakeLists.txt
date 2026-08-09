add_executable(ulam_prime_spiral src/main.cpp)
target_compile_features(ulam_prime_spiral PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(ulam_prime_spiral PRIVATE /W4 /permissive-)
else()
  target_compile_options(ulam_prime_spiral PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  add_test(
    NAME artifact_002_smoke
    COMMAND ulam_prime_spiral
      --output ${CMAKE_CURRENT_BINARY_DIR}/artifact-002-smoke.svg
      --side 101
      --width 640
      --height 640
  )
  set_tests_properties(artifact_002_smoke PROPERTIES
    PASS_REGULAR_EXPRESSION "Rendered 1252 primes"
  )

  add_test(NAME artifact_002_rejects_even_side
    COMMAND ulam_prime_spiral --side 100)
  set_tests_properties(artifact_002_rejects_even_side PROPERTIES WILL_FAIL TRUE)
endif()
