set(qmpmpris_SOURCES
    qmpmpris.hpp
    qmprisdbusinterface.hpp
    qmpriswrapper.hpp
    qmpmprisimpl.hpp
    qmpmpris.cpp
    qmprisdbusinterface.cpp
    qmpriswrapper.cpp
    qmpmprisimpl.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS DBus)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(qmpmpris MODULE
    ${qmpmpris_SOURCES}
)

target_link_libraries(qmpmpris
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
)

install(TARGETS qmpmpris LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qmidiplayer/)
