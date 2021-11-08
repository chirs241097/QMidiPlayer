add_library(backtrace SHARED backtrace.c)
find_path(BFD_INCLUDE_DIR NAMES bfd.h)
include_directories(${BFD_INCLUDE_DIR})
find_library(BFD_LIBRARIES NAMES bfd)
find_library(IBERTY_LIBRARIES NAMES iberty)
find_library(INTL_LIBRARIES NAMES intl)
find_package(ZLIB REQUIRED)
set_target_properties(backtrace PROPERTIES PREFIX "")
target_link_libraries(backtrace
	${BFD_LIBRARIES}
	${IBERTY_LIBRARIES}
	${INTL_LIBRARIES}
	${ZLIB_LIBRARIES}
	imagehlp
)
