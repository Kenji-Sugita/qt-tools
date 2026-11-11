qt_add_executable(AnkerFormLayoutTest
    tst_formlayout.cpp
)

target_link_libraries(AnkerFormLayoutTest
    PRIVATE
        AnkerLayout
        Qt6::Test
)

add_test(NAME AnkerFormLayoutTest COMMAND AnkerFormLayoutTest)
