add_executable(ThaiShapingTest
  ThaiShapingTest.cpp
  ${REPO_ROOT}/lib/ThaiShaping/ThaiShaping.cpp
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
)

target_include_directories(ThaiShapingTest PRIVATE
  ${REPO_ROOT}/lib/ThaiShaping
  ${REPO_ROOT}/lib/Utf8
)

target_link_libraries(ThaiShapingTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ThaiShapingTest)
