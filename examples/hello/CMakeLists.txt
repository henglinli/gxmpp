aux_source_directory(. hello_SRC)
add_executable(gxmpp_hello ${hello_SRC})
target_link_libraries(gxmpp_hello gxmpp pthread rt)