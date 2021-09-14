aux_source_directory(. DIR_EXTENSIONS_SRCS)
include_directories(../core)
add_library(extensions ${DIR_EXTENSIONS_SRCS})
target_link_libraries(extensions PRIVATE pthread)