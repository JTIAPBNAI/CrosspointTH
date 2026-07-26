add_executable(ThaiBreakingTest
  ThaiBreakingTest.cpp
  ${REPO_ROOT}/lib/ThaiBrk/ThaiBrk.cpp
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
)

target_include_directories(ThaiBreakingTest PRIVATE
  ${REPO_ROOT}/lib/ThaiBrk
  ${REPO_ROOT}/lib/Utf8
)

target_link_libraries(ThaiBreakingTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ThaiBreakingTest)
