add_executable(MarkdownParserTest
  MarkdownParserTest.cpp
  ${REPO_ROOT}/lib/Txt/MarkdownParser.cpp
)

target_include_directories(MarkdownParserTest PRIVATE
  ${REPO_ROOT}/lib/Txt
  ${REPO_ROOT}/lib/EpdFont
)

target_link_libraries(MarkdownParserTest PRIVATE
  GTest::gtest_main
)

gtest_discover_tests(MarkdownParserTest)
