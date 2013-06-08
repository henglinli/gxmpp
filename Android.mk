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
	$(LOCAL_PATH)/third_party/expat/lib

# gxmpp
# =====================================================

gxmpp_SRC_FILES := \
	$(wildcard talk/base/*.cc) \
	$(wildcard talk/xmllite/*.cc) \
	$(wildcard talk/xmpp/*.cc)

gxmpp_CXXFLAGS := \
	-Wall \
	-fPIC \
	-DPOSIX \
	-DLINUX \
	-DEXPAT_RELATIVE_PATH

gxmpp_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/third_party/expat/lib

# build
# =====================================================

# expat static library
include $(CLEAR_VARS)

LOCAL_MODULE := expat_static

LOCAL_C_INCLUDES += $(expat_C_INCLUDES)

LOCAL_CFLAGS += $(expat_CFLAGS)

LOCAL_SRC_FILES := $(expat_SRC_FILES)

include $(BUILD_STATIC_LIBRARY)

# gxmpp static library
include $(CLEAR_VARS)

LOCAL_MODULE := gxmpp_static

LOCAL_C_INCLUDES += $(gxmpp_C_INCLUDES)

#LOCAL_CFLAGS := -std=gnu++11

LOCAL_CXXFLAGS += $(gxmpp_CXXFLAGS)

LOCAL_SRC_FILES := $(gxmpp_SRC_FILES)

include $(BUILD_STATIC_LIBRARY)

# gxmpp shared library
include $(CLEAR_VARS)

LOCAL_MODULE := gxmpp

LOCAL_C_INCLUDES += $(gxmpp_C_INCLUDES)

LOCAL_CXXFLAGS += $(gxmpp_CXXFLAGS)

LOCAL_SRC_FILES := $(wildcard examples/echo/*.cc) 

LOCAL_STATIC_LIBRARIES := gxmpp_static expat_static  

include $(BUILD_SHARED_LIBRARY)
