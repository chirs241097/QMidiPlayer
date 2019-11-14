set(qmpdesktop_SOURCES
    qdialskulpturestyle.hpp
    qmpchanneleditor.hpp
    qmpchannelswindow.hpp
    qmpcustomizewindow.hpp
    qmpdeviceprioritydialog.hpp
    qmpdevpropdialog.hpp
    qmpefxwindow.hpp
    qmphelpwindow.hpp
    qmpinfowindow.hpp
    qmpmainwindow.hpp
    qmpplistwindow.hpp
    qmpplugin.hpp
    qmppresetselect.hpp
    qmpsettingswindow.hpp
    main.cpp
    qdialskulpturestyle.cpp
    qmpchanneleditor.cpp
    qmpchannelswindow.cpp
    qmpcustomizewindow.cpp
    qmpdeviceprioritydialog.cpp
    qmpdevpropdialog.cpp
    qmpefxwindow.cpp
    qmphelpwindow.cpp
    qmpinfowindow.cpp
    qmpmainwindow.cpp
    qmpplistwindow.cpp
    qmpplugin.cpp
    qmppresetselect.cpp
    qmpsettingswindow.cpp
    qmpchanneleditor.ui
    qmpchannelswindow.ui
    qmpcustomizewindow.ui
    qmpdeviceprioritydialog.ui
    qmpdevpropdialog.ui
    qmpefxwindow.ui
    qmphelpwindow.ui
    qmpinfowindow.ui
    qmpmainwindow.ui
    qmpplistwindow.ui
    qmppresetselect.ui
    qmpsettingswindow.ui
    resources.qrc
)

if(WIN32)
    set(qmpdesktop_SOURCES
        ${qmpdesktop_SOURCES}
        qmidiplayer.rc
    )
endif(WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${fluidsynth_INCLUDE_DIRS})
include_directories(${rtmidi_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include/)

cmake_host_system_information(RESULT build_host QUERY HOSTNAME)
add_definitions(-DBUILD_MACHINE=${build_host})

add_executable(qmidiplayer
    ${qmpdesktop_SOURCES}
)

target_link_libraries(qmidiplayer
    Qt5::Widgets
    qmpcore
    ${fluidsynth_LIBRARIES}
    ${rtmidi_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)
