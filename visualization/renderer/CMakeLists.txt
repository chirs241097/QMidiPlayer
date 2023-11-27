set(qmpvisrender_SOURCES
	qmpsettingsro.hpp
	qmppluginapistub.hpp
	qmpvisrendercore.hpp
	main.cpp
	qmpsettingsro.cpp
	qmppluginapistub.cpp
	qmpvisrendercore.cpp
)

set(CMAKE_AUTOMOC ON)

include_directories(${PROJECT_SOURCE_DIR}/core/)
include_directories(${PROJECT_SOURCE_DIR}/include/)

add_executable(qmpvisrender
    ${qmpvisrender_SOURCES}
)

target_link_libraries(qmpvisrender
    Qt${QT_VERSION_MAJOR}::Core
    qmpcore
    ICU::uc
    ICU::i18n
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

if(WIN32)
    target_link_libraries(qmpvisrender winmm)
endif()

install(TARGETS qmpvisrender)
