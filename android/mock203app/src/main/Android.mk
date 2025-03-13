LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_PACKAGE_NAME := org.orbtv.mock203app

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_STATIC_JAVA_LIBRARIES += \
   org.orbtv.companionlibrary \
   org.orbtv.orblibrary \
   android-support-v17-leanback \
   okhttp3.mockwebserver \
   junit

DIAL_AIDL_PATH := ../../../mockdialservice/src/main/aidl

LOCAL_SRC_FILES := \
   $(call all-subdir-java-files) \
   $(DIAL_AIDL_PATH)/org/orbtv/mockdialservice/IMockDialService.aidl \
   $(DIAL_AIDL_PATH)/org/orbtv/mockdialservice/IMockDialServiceCallback.aidl


LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/$(DIAL_AIDL_PATH)

LOCAL_JNI_SHARED_LIBRARIES := \
   liborg.orbtv.orblibrary.native

LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

# Define the source and destination directories
TESTS_SRC := $(LOCAL_PATH)/../../../../tests
TESTS_DST := $(LOCAL_PATH)/assets/tests

# Pre-build step to copy files before compiling
$(shell mkdir -p $(TESTS_DST))  # Ensure destination exists
$(shell cp -r $(TESTS_SRC)/* $(TESTS_DST)/)

include $(BUILD_PACKAGE)
###############################
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := okhttp3.mockwebserver:libs/mockwebserver-3.14.9.jar
include $(BUILD_MULTI_PREBUILT)
###############################
