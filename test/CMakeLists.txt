cmake_minimum_required(VERSION 3.5)
project(test)
aux_source_directory(. DIR_TEST_SRCS)

add_subdirectory(../core ./core)
add_subdirectory(../extensions ./extensions)

include_directories(
    ../core
    ../extensions
)

set(TEST_LIBS
        core
        extensions
)

add_executable(main ${DIR_TEST_SRCS})
target_link_libraries(main ${TEST_LIBS})
