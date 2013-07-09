set(expat_SRC expat/lib/xmlparse.c expat/lib/xmlrole.c expat/lib/xmltok.c)
add_definitions(-DHAVE_MEMMOVE)
add_library(expat OBJECT ${expat_SRC})
