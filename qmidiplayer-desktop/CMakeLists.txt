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
    qmpsettings.hpp
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
    qmpsettings.cpp
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
    get_filename_component(CURRENT_SUBPROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    configure_file("qmidiplayer.rc.in" "qmidiplayer.rc" @ONLY)
    list(APPEND qmpdesktop_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qmidiplayer.rc)
endif(WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${fluidsynth_INCLUDE_DIRS})
include_directories(${rtmidi_INCLUDE_DIRS})
include_directories(${ICU_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/core/)

cmake_host_system_information(RESULT build_host QUERY HOSTNAME)
add_definitions(-DBUILD_MACHINE=${build_host})
if(UNIX AND NOT BUILD_PORTABLE)
	add_definitions(-DNON_PORTABLE -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
endif()

add_executable(qmidiplayer
    ${qmpdesktop_SOURCES}
)

target_link_libraries(qmidiplayer
    Qt${QT_VERSION_MAJOR}::Widgets
    qmpcore
    ICU::uc
    ICU::i18n
    ${fluidsynth_LIBRARIES}
    ${rtmidi_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

if(WIN32)
    target_link_libraries(qmidiplayer winmm)
endif()

target_link_directories(qmidiplayer
PRIVATE
    ${fluidsynth_LIBRARY_DIRS}
    ${rtmidi_LIBRARY_DIRS}
)

set(qmpdesktop_TS_FILES
    translations/qmp_zh_CN.ts
)
if(${QT_VERSION_MAJOR} EQUAL "5")
    qt_create_translation(qmpdesktop_QM_FILES ${qmpdesktop_SOURCES} ${qmpdesktop_TS_FILES})
    add_custom_target(translations ALL DEPENDS ${qmpdesktop_QM_FILES})
else()
    qt_add_translations(qmidiplayer TS_FILES ${qmpdesktop_TS_FILES} QM_FILES_OUTPUT_VARIABLE qmpdesktop_QM_FILES)
endif()

install(TARGETS qmidiplayer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/doc DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qmidiplayer)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/img DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qmidiplayer FILES_MATCHING PATTERN "*.png")
install(FILES org.chrisoft.qmidiplayer.appdata.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo)
install(FILES org.chrisoft.qmidiplayer.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES ${PROJECT_SOURCE_DIR}/img/qmidiplyr.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps RENAME org.chrisoft.qmidiplayer.png)
install(FILES ${PROJECT_SOURCE_DIR}/img/qmidiplyr.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME org.chrisoft.qmidiplayer.svg)
install(FILES menu/qmidiplayer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/menu)
install(FILES qmidiplayer.mime DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)
install(FILES ${qmpdesktop_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qmidiplayer/translations)
