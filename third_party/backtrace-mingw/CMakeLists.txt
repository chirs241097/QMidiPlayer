add_library(backtrace SHARED backtrace.c)
find_path(BFD_INCLUDE_DIR NAMES bfd.h)
find_library(BFD_LIBRARIES NAMES bfd)
find_library(IBERTY_LIBRARIES NAMES iberty)
find_package(ZLIB REQUIRED)
target_link_libraries(backtrace
	${BFD_LIBRARIES}
	${IBERTY_LIBRARIES}
	${ZLIB_LIBRARIES}
	imagehlp
)
