set(midifmt-plugin_SOURCES
    midifmtplugin.hpp
    midifmtplugin.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(midifmt-plugin MODULE
    ${midifmt-plugin_SOURCES}
)
