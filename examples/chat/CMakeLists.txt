aux_source_directory(. chat_SRC)
add_executable(gxmpp_chat ${chat_SRC})
target_link_libraries(gxmpp_chat gxmpp pthread rt)