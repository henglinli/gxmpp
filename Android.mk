LOCAL_PATH:= $(call my-dir)

# expat 
# =====================================================
expat_SRC_FILES := \
	third_party/expat/lib/xmlparse.c \
	third_party/expat/lib/xmlrole.c \
	third_party/expat/lib/xmltok.c

expat_CFLAGS := \
	-Wall \
        -Wmissing-prototypes -Wstrict-prototypes \
        -Wno-unused-parameter -Wno-missing-field-initializers \
	-fPIC \
        -DHAVE_MEMMOVE

expat_C_INCLUDES += \
	$(LOCAL_PATH)/lib

# gxmpp
# =====================================================

gxmpp_SRC_FILES := \
	$(wildcard ./talk/base/*.cc) \
        $(wildcard ./talk/xmllite/*.cc) \
        $(wildcard ./talk/xmpp/*.cc)

gxmpp_CXXFLAGS := \
	-Wall \
	-fPIC \
	-DPOSIX \
	-DLINUX \
	-DEXPAT_RELATIVE_PATH

gxmpp_C_INCLUDES += \
	$(LOCAL_PATH)

# build
# =====================================================

# expat static library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(expat_SRC_FILES)
LOCAL_CFLAGS += $(expat_CFLAGS)
LOCAL_C_INCLUDES += $(expat_C_INCLUDES)

LOCAL_MODULE:= libexpat_static

include $(BUILD_STATIC_LIBRARY)

# expat static library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(gxmpp_SRC_FILES)
LOCAL_CXXFLAGS += $(gxmpp_CXXFLAGS)
LOCAL_C_INCLUDES += $(gxmpp_C_INCLUDES)

LOCAL_MODULE:= libgxmpp_static

include $(BUILD_STATIC_LIBRARY)

# gxmpp shared library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := 
LOCAL_CXXFLAGS += 
LOCAL_C_INCLUDES += 

LOCAL_STATIC_LIBRARIES := libexpat_static libgxmpp_static 

LOCAL_MODULE:= libgxmpp

include $(BUILD_SHARED_LIBRARY)
