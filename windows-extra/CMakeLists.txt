set(windowsextra_SOURCES
    windowsextra.hpp
    windowsextra.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include/)

find_package(Qt5 REQUIRED COMPONENTS Widgets WinExtras)

add_library(windowsextra MODULE
    ${windowsextra_SOURCES}
)

target_link_libraries(windowsextra Qt5::Widgets Qt5::WinExtras)

install(TARGETS windowsextra LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
