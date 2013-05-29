# Copyright (c) 2013 henglinli@gmail.com. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

#-----------------------------------------------
# Uncomment exactly one of the lines labelled (A), (B), and (C) below
# to switch between compilation modes.

OPT ?= -O2 -DNDEBUG 	  # (A) Production use (optimized mode)
# OPT ?= -g2              # (B) Debug mode, w/ full line-level debugging symbols
# OPT ?= -O2 -g2 -DNDEBUG # (C) Profiling mode: opt, but w/debugging symbols
#-----------------------------------------------
CC = clang
CXX = clang++

FLAGS += -DPOSIX -DLINUX -DEXPAT_RELATIVE_PATH 
CFLAGS += -I. -I./third_party/expat/lib  $(OPT) $(FLAGS) -fPIC -DHAVE_MEMMOVE
CXXFLAGS += -I. -I./third_party/expat/lib  $(OPT) $(FLAGS) -fPIC

LDFLAGS += -pthread
LIBS += -lrt

EXPAT_SRC += third_party/expat/lib/xmlparse.c \
	third_party/expat/lib/xmlrole.c \
	third_party/expat/lib/xmltok.c

EXPAT_OBJS += $(EXPAT_SRC:.c=.o)

SOURCES += $(wildcard ./talk/base/*.cc) \
	$(wildcard ./talk/xmllite/*.cc) \
	$(wildcard ./talk/xmpp/*.cc)

LIBOBJECTS = $(SOURCES:.cc=.o) $(EXPAT_OBJS)

PROGRAMS = gxmpp_hello

LIBRARY = libgxmpp.a

SHARED = libgxmpp.so

default: all

all: $(LIBRARY) $(SHARED) $(PROGRAMS)

clean:
	-rm -f $(PROGRAMS) $(LIBRARY) $(SHARED) $(LIBOBJECTS)

$(SHARED): $(LIBOBJECTS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -shared -o $(SHARED) 


$(LIBRARY): $(LIBOBJECTS)
	rm -f $@
	$(AR) -rs $@ $(LIBOBJECTS)

HELLO_SRC += $(wildcard ./example/*.cc)
HELLO_OBJS += $(HELLO_SRC:.cc=.o)
gxmpp_hello: $(LIBOBJECTS) $(HELLO_OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@ 

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
