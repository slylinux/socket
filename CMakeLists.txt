aux_source_directory(. srcs)
add_executable(echo ${srcs})
target_link_libraries(echo pthread)
