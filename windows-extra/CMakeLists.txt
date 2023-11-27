set(windowsextra_SOURCES
    windowsextra.hpp
    windowsextra.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(windowsextra MODULE
    ${windowsextra_SOURCES}
)

target_link_libraries(windowsextra Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::WinExtras)

install(TARGETS windowsextra LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
