set(sampleplugin_SOURCES
    sampleplugin.hpp
    sampleplugin.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(sampleplugin MODULE
    ${sampleplugin_SOURCES}
)
