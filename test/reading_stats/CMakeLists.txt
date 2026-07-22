add_executable(ReadingStatsCodecTest ReadingStatsCodecTest.cpp)

target_include_directories(ReadingStatsCodecTest PRIVATE
  ${REPO_ROOT}/src/activities/reader
)

target_link_libraries(ReadingStatsCodecTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ReadingStatsCodecTest)
