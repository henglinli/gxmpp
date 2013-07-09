add_definitions(-std=c++11)
aux_source_directory(. extra_SRC)
add_library(extra OBJECT ${extra_SRC})
