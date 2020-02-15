set(midifmt-plugin_SOURCES
    midifmtplugin.hpp
    midifmtplugin.cpp
)

if(WIN32)
    get_filename_component(CURRENT_SUBPROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    configure_file("midifmt-plugin.rc.in" "midifmt-plugin.rc" @ONLY)
    list(APPEND midifmt-plugin_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/midifmt-plugin.rc)
endif(WIN32)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(midifmt-plugin MODULE
    ${midifmt-plugin_SOURCES}
)

install(TARGETS midifmt-plugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
