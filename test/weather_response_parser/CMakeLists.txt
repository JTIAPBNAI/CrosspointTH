add_executable(WeatherResponseParserTest
  WeatherResponseParserTest.cpp
  ${REPO_ROOT}/src/activities/apps/WeatherResponseParser.cpp
)

target_include_directories(WeatherResponseParserTest PRIVATE
  ${REPO_ROOT}/src/activities/apps
)

target_link_libraries(WeatherResponseParserTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(WeatherResponseParserTest)
