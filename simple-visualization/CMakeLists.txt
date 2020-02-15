set(simple-visualization_SOURCES
    qmpkeyboardwindow.hpp
    qmppianowidget.hpp
    simplevisualization.hpp
    qmpkeyboardwindow.cpp
    qmppianowidget.cpp
    simplevisualization.cpp
)

if(WIN32)
    list(APPEND simple-visualization_SOURCES simple-visualization.rc)
endif(WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(simple-visualization MODULE
    ${simple-visualization_SOURCES}
)

target_link_libraries(simple-visualization
    Qt5::Widgets
)

install(TARGETS simple-visualization LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
