add_definitions(-DEXPAT_RELATIVE_PATH)
aux_source_directory(. talk_xmpp_SRC)
add_library(talk_xmpp OBJECT ${talk_xmpp_SRC})
