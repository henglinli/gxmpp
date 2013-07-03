# Copyright (c) 2013 henglinli@gmail.com. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

#-----------------------------------------------
# Uncomment exactly one of the lines labelled (A), (B), and (C) below
# to switch between compilation modes.

OPT ?= -g -O0 -pipe -DLOGGING # (A) Production use (optimized mode)
# OPT ?= -g2              # (B) Debug mode, w/ full line-level debugging symbols
# OPT ?= -O2 -g2 -DNDEBUG # (C) Profiling mode: opt, but w/debugging symbols
#-----------------------------------------------
CC = clang
CXX = clang++

FLAGS += -DPOSIX -DEXPAT_RELATIVE_PATH -Wall -Wextra -fPIC 
CFLAGS += -I. -Ithird_party/expat/lib  $(OPT) $(FLAGS) -DHAVE_MEMMOVE
CXXFLAGS += -I. -Ithird_party/expat/lib  $(OPT) $(FLAGS) -std=c++11
LDFLAGS += 
LIBS += 

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	FLAGS += -DLINUX
	LDFLAGS += -pthread
	LIBS += -lrt
endif
ifeq ($(UNAME_S),Darwin)
    FLAGS += -DOSX -fobjc-arc -stdlib=libc++
	LDFLAGS += -lc++
	LIBS +=  -lobjc -framework CoreFoundation -framework Cocoa
	OSX_SRC += $(wildcard talk/base/*.mm)
endif


EXPAT_SRC += third_party/expat/lib/xmlparse.c \
	third_party/expat/lib/xmlrole.c \
	third_party/expat/lib/xmltok.c

EXPAT_OBJS += $(EXPAT_SRC:.c=.o)

SOURCES += $(wildcard talk/base/*.cc) \
	$(wildcard talk/xmllite/*.cc) \
	$(wildcard talk/xmpp/*.cc) \
	$(wildcard gxmpp/*.cc)

COMMON_OBJS += $(SOURCES:.cc=.o)

OSX_OBJS += $(OSX_SRC:.mm=.o)

LIBOBJECTS = $(COMMON_OBJS) $(OSX_OBJS) $(EXPAT_OBJS)

PROGRAMS = gxmpp_hello gxmpp_chat gxmpp_echo

LIBRARY = libgxmpp.a

SHARED = libgxmpp.so

default: all

all: $(LIBRARY) $(SHARED) $(PROGRAMS)

clean:
	-rm -f $(PROGRAMS) $(LIBRARY) $(SHARED) $(LIBOBJECTS) $(HELLO_OBJS) $(CHAT_OBJS) $(ECHO_OBJS)

$(SHARED): $(LIBOBJECTS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -shared -o $(SHARED) 


$(LIBRARY): $(LIBOBJECTS)
	rm -f $@
	$(AR) -rs $@ $(LIBOBJECTS)

HELLO_SRC += $(wildcard ./examples/hello/*.cc)
HELLO_OBJS += $(HELLO_SRC:.cc=.o)
gxmpp_hello: $(HELLO_OBJS) $(LIBRARY)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@ 

CHAT_SRC += $(wildcard ./examples/chat/*.cc)
CHAT_OBJS += $(CHAT_SRC:.cc=.o)
gxmpp_chat: $(CHAT_OBJS) $(LIBRARY)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@ 

ECHO_SRC += $(wildcard ./examples/echo/*.cc) 
ECHO_SRC_MAIN += $(wildcard ./examples/echo/*.cpp)
ECHO_OBJS_MAIN += $(ECHO_SRC_MAIN:.cpp=.o)
ECHO_OBJS += $(ECHO_SRC:.cc=.o) $(ECHO_OBJS_MAIN)

gxmpp_echo: $(ECHO_OBJS) $(LIBRARY)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@ 

#=============================================================
%.o : %.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
