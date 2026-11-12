qt_add_executable(Pattern10SubdirTest
    tst_validation.cpp
)

target_link_libraries(Pattern10SubdirTest
    PRIVATE
        Qt6::Test
)

add_test(NAME Pattern10SubdirTest COMMAND Pattern10SubdirTest)
