set(visualization_SOURCES
    extrasmeltutils.hpp
    qmpvirtualpiano3d.hpp
    qmpvisualization.hpp
    extrasmeltutils.cpp
    qmpvirtualpiano3d.cpp
    qmpvisualization.cpp
)

if(WIN32)
    get_filename_component(CURRENT_SUBPROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    configure_file("visualization.rc.in" "visualization.rc" @ONLY)
    list(APPEND visualization_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/visualization.rc)
endif(WIN32)

option(BUILD_DUMB "SMELT override: disable sound support" ON)
option(BUILD_EXAMPLE "SMELT override: build example" OFF)
add_subdirectory(SMELT)

add_subdirectory(renderer)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SMELT/include)
include_directories(${FREETYPE_INCLUDE_DIRS})

add_library(visualization MODULE
    ${visualization_SOURCES}
)

target_link_libraries(visualization
    smeltext
    smelt
    ${FREETYPE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${IL_LIBRARIES}
    glfw
    GLEW::glew
    OpenGL::GL
)

install(TARGETS visualization LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
