qt_add_executable(Pattern10SubdirApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern10SubdirApp
    PRIVATE
        Qt6::Widgets
)
