LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../src \
    $(LOCAL_PATH)/../src/include \

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/src/,, $(wildcard $(LOCAL_PATH)/../src/*.cpp))

LOCAL_MODULE    := testenc
LOCAL_MODULE_TAGS := optional

APP_CFLAGS := -DHAVE_LIBRESOLV= \
		-DHAVE_PTHREAD=1 \
		-DHAVE_PTHREAD_RWLOCK=1 \
		-DHAVE_LIBPTHREAD= \
		-DHAVE_EPOLL \
		-DPEDANTIC= \
		-DUSE_OPENSSL=yes \
		-DANDROID=1 \
		-std=gnu99 \
		-DUSE_VIDEO=1 \
		-DHAVE_INET6 \
		-DSTATIC=1

LOCAL_CFLAGS := $(APP_CFLAGS)

LOCAL_LDLIBS := \
    ../libs/libvpu.so \
    ../libs/librk_on2.so

include $(BUILD_EXECUTABLE)
