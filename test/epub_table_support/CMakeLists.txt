add_executable(EpubTableSupportTest
  EpubTableSupportTest.cpp
)

target_include_directories(EpubTableSupportTest PRIVATE
  ${REPO_ROOT}/lib/Epub/Epub/parsers
)

target_link_libraries(EpubTableSupportTest PRIVATE
  GTest::gtest_main
)

gtest_discover_tests(EpubTableSupportTest)
