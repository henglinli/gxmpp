if(UNIX)
  if(APPLE)
    aux_source_directory(. talk_base_SRC)
  else()
    file(GLOB talk_base_SRC "*.cc")
  endif()
endif()
add_library(talk_base OBJECT ${talk_base_SRC})
