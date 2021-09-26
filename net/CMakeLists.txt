aux_source_directory(. DIR_NET_SRCS)
add_library(net ${DIR_NET_SRCS})

include_directories(
    ../core
    ../extensions
)

target_link_libraries(net PRIVATE core extensions)