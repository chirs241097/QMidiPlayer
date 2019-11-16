set(sampleplugin_SOURCES
    sampleplugin.hpp
    sampleplugin.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(sampleplugin MODULE
    ${sampleplugin_SOURCES}
)

install(TARGETS sampleplugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
