add_definitions(-DEXPAT_RELATIVE_PATH)
aux_source_directory(. talk_xmllite_SRC)
add_library(talk_xmllite OBJECT ${talk_xmllite_SRC})