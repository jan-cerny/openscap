
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release)
    set(TARGET_DIR "release")
endif ()

set(RUSTOSCAP_SO "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/librustoscap.so")

add_custom_target(rustoscap ALL
    COMMENT "Compiling Rust module"
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} ${CARGO_CMD}
    COMMAND cp ${RUSTOSCAP_SO} ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(rustoscap PROPERTIES LOCATION ${CMAKE_CURRENT_BINARY_DIR})
