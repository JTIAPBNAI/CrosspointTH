add_executable(ThaiDraughtsEngineTest
  ThaiDraughtsEngineTest.cpp
  ${REPO_ROOT}/src/activities/apps/games/ThaiDraughtsEngine.cpp
)

target_include_directories(ThaiDraughtsEngineTest PRIVATE
  ${REPO_ROOT}/src/activities/apps/games
)

target_link_libraries(ThaiDraughtsEngineTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ThaiDraughtsEngineTest)
