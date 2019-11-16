set(visualization_SOURCES
    extrasmeltutils.hpp
    qmpvirtualpiano3d.hpp
    qmpvisualization.hpp
    extrasmeltutils.cpp
    qmpvirtualpiano3d.cpp
    qmpvisualization.cpp
)

set(BUILD_DUMB ON)
set(BUILD_EXAMPLE OFF)
add_subdirectory(SMELT)

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
    ${GLEW_LIBRARIES}
    OpenGL::GL
)

install(TARGETS visualization LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
