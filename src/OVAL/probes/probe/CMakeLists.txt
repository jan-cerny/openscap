file(GLOB_RECURSE PROBE_SOURCES "*.c")
file(GLOB_RECURSE PROBE_HEADERS "*.h")
list(REMOVE_ITEM PROBE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
add_library(probe_object OBJECT ${PROBE_SOURCES} ${PROBE_HEADERS})
target_include_directories(probe_object PUBLIC
	"${CMAKE_SOURCE_DIR}/src/OVAL/probes"
	"${CMAKE_SOURCE_DIR}/src/OVAL/probes/public")
set_oscap_generic_properties(probe_object)
