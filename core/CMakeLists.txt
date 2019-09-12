set(qmpcore_SOURCES
    qmpmidioutfluid.hpp
    qmpmidioutfluid.cpp
    qmpmidioutrtmidi.hpp
    qmpmidioutrtmidi.cpp
    qmpmidiplay.hpp
    qmpmidiplay.cpp
    qmpmidiread.cpp
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${fluidsynth_INCLUDE_DIRS})
include_directories(${rtmidi_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(qmpcore STATIC
    ${qmpcore_SOURCES}
)
