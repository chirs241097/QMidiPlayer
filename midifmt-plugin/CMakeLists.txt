set(midifmt-plugin_SOURCES
    midifmtplugin.hpp
    midifmtplugin.cpp
)

if(WIN32)
    list(APPEND midifmt-plugin_SOURCES midifmt-plugin.rc)
endif(WIN32)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(midifmt-plugin MODULE
    ${midifmt-plugin_SOURCES}
)

install(TARGETS midifmt-plugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
