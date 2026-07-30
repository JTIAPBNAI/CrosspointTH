add_executable(FlashcardScheduleTest
  FlashcardScheduleTest.cpp
  ${REPO_ROOT}/src/activities/apps/flashcards/FlashcardSchedule.cpp
)

target_include_directories(FlashcardScheduleTest PRIVATE
  ${REPO_ROOT}/src/activities/apps/flashcards
)

target_link_libraries(FlashcardScheduleTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(FlashcardScheduleTest)
