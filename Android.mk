# Copyright (C) 2012 Mozilla Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)

gonk_misc_LOCAL_PATH := $(LOCAL_PATH)
include $(call all-subdir-makefiles)
LOCAL_PATH := $(gonk_misc_LOCAL_PATH)


include $(CLEAR_VARS)
LOCAL_MODULE       := init.b2g.rc
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := init.b2g.rc
LOCAL_MODULE_PATH  := $(TARGET_ROOT_OUT)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := b2g.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES    := b2g.sh
LOCAL_MODULE_PATH  := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := api-daemon.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES    := api-daemon.sh
LOCAL_MODULE_PATH  := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := updater-daemon.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES    := updater-daemon.sh
LOCAL_MODULE_PATH  := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := b2g-ps
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES    := b2g-ps
LOCAL_MODULE_PATH  := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := b2g-procrank
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES    := b2g-procrank
LOCAL_MODULE_PATH  := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

#
# Dhcpcd
#
include $(CLEAR_VARS)
LOCAL_MODULE := dhcpcd.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/dhcpcd-6.8.2
LOCAL_SRC_FILES := dhcpcd/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := dhcpcd-run-hooks
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/dhcpcd-6.8.2
LOCAL_SRC_FILES := dhcpcd/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := 20-dns.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/dhcpcd-6.8.2/dhcpcd-hooks
LOCAL_SRC_FILES := dhcpcd/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := 95-configured
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/dhcpcd-6.8.2/dhcpcd-hooks
LOCAL_SRC_FILES := dhcpcd/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fakeappops
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES    := fakeappops.cpp
LOCAL_CFLAGS := -DANDROID_VERSION=$(PLATFORM_SDK_VERSION)
LOCAL_SHARED_LIBRARIES := libbinder libutils
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE       := gonksched
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES    := gonksched.cpp
LOCAL_SHARED_LIBRARIES := libbinder libutils libcutils libmediautils libprocessgroup
LOCAL_CFLAGS := -DANDROID_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_C_INCLUDES := frameworks/av/media/utils
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE       := gonkservices
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES    := gonkservices.cpp
LOCAL_SHARED_LIBRARIES := libbinder libutils libsensorprivacy
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE	   := b2gkillerd
LOCAL_MODULE_TAGS  := optional
LCOAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES	   := b2gkillerd.cpp
LOCAL_SHARED_LIBRARIES := libcutils liblog
include $(BUILD_EXECUTABLE)

#
# Gecko glue
#

include $(CLEAR_VARS)
GECKO_PATH ?= gecko
ifeq (,$(GECKO_OBJDIR))
GECKO_OBJDIR := $(TARGET_OUT_INTERMEDIATES)/objdir-gecko
endif

-include prebuilt-b2g/Android.mk
LOCAL_MODULE := gecko
LOCAL_MODULE_CLASS := DATA
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

B2G_SYSTEM_APPS ?= 0
USE_PREBUILT_B2G ?= 0
# Preserve the /system/b2g/defaults as its contents will be populated before this rule.
PRESERVE_DIRS := defaults
ifeq ($(B2G_SYSTEM_APPS), 1)
PRESERVE_DIRS += webapps
endif

$(LOCAL_INSTALLED_MODULE) : $(LOCAL_BUILT_MODULE)
	@echo Install dir: $(TARGET_OUT)/b2g
	rm -rf $(filter-out $(addprefix $(TARGET_OUT)/b2g/,$(PRESERVE_DIRS)),$(wildcard $(TARGET_OUT)/b2g/*))
	mkdir -p $(TARGET_OUT)/b2g/defaults/pref
	cd $(TARGET_OUT) && tar jxvf $(abspath $<)
ifneq ($(EXPORT_DEVICE_PREFS),)
	cp -n $(EXPORT_DEVICE_PREFS)/*.js $(TARGET_OUT)/b2g/defaults/pref/
endif
ifneq ($(EXPORT_DEVICE_USER_BUILD_PREFS),)
	cp -n $(EXPORT_DEVICE_USER_BUILD_PREFS) $(TARGET_OUT)/b2g/defaults/pref/
endif
ifneq ($(EXPORT_BUILD_PREFS),)
	cp -n $(EXPORT_BUILD_PREFS) $(TARGET_OUT)/b2g/defaults/pref/
endif

GECKO_LIB_DEPS := \
	liblog.so \
	libmedia.so \
	libmtp.so \
	libsensorservice.so \
	libstagefright.so \
	libstagefright_omx.so \
	libsysutils.so \
	android.hardware.gnss@1.0.so \
	android.hardware.graphics.composer@2.1.so \
	android.hardware.graphics.composer@2.2.so \
	android.hardware.graphics.composer@2.3.so \
	android.hardware.power@1.0.so \
	android.hardware.radio@1.0.so \
	android.hardware.vibrator@1.0.so \
	android.hardware.sensors@1.0.so \
	android.hardware.wifi@1.0.so \
	android.hardware.wifi@1.1.so \
	android.hardware.wifi@1.2.so \
	android.hardware.wifi@1.3.so \
	android.hardware.wifi.hostapd@1.0.so \
	android.hardware.wifi.hostapd@1.1.so \
	android.hardware.wifi.supplicant@1.0.so \
	android.hardware.wifi.supplicant@1.1.so \
	android.hardware.wifi.supplicant@1.2.so \
	android.system.wifi.keystore@1.0.so \
	libwificond_ipc_shared.so \
	netd_aidl_interface-V2-cpp.so \
	netd_event_listener_interface-V1-cpp.so \
	dnsresolver_aidl_interface-V2-cpp.so \
	binder_b2g_connectivity_interface-cpp.so \
	binder_b2g_telephony_interface-cpp.so \
	libc++.so \
	libbinder.so \
	libfmq.so \
	libutils.so \
	libcutils.so \
	libhardware_legacy.so \
	libhardware.so \
	libui.so \
	libgui.so \
	libsuspend.so \
	libsync.so \
	libhidlbase.so \
	libvold_binder_shared.so \
	libttspico.so \
	$(NULL)

# For APEX_MODULE_LIBS
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) < 29 ))" )))
GECKO_LIB_DEPS += \
	libc.so \
	libdl.so \
	libm.so \
	$(NULL)
else
GECKO_LIB_DEPS += \
	android.hardware.gnss.visibility_control@1.0.so \
	android.hardware.gnss@1.1.so \
	android.hardware.gnss@2.0.so \
	$(NULL)
endif

ifeq ($(IME_ENGINE), touchpal)
GECKO_LIB_DEPS += libtouchpal.so
endif
PRODUCTION_OS_NAME ?= KAIOS

.PHONY: $(LOCAL_BUILT_MODULE)
$(LOCAL_BUILT_MODULE): $(TARGET_CRTBEGIN_DYNAMIC_O) $(TARGET_CRTEND_O) $(addprefix $(TARGET_OUT_SHARED_LIBRARIES)/,$(GECKO_LIB_DEPS)) $(GECKO_LIB_STATIC)
ifeq ($(USE_PREBUILT_B2G),1)
	@echo -e "\033[0;33m ==== Use prebuilt gecko ==== \033[0m";
	mkdir -p $(@D) && cp $(abspath $(PREFERRED_B2G)) $@
else
	(echo "export GECKO_OBJDIR=$(abspath $(GECKO_OBJDIR))"; \
	echo "export GONK_PRODUCT_NAME=$(TARGET_DEVICE)"; \
	echo "export GONK_PATH=$(abspath .)"; \
	echo "export PLATFORM_VERSION=$(PLATFORM_SDK_VERSION)"; \
	echo "export TARGET_ARCH=$(TARGET_ARCH)"; \
	echo "export TARGET_ARCH_VARIANT=$(TARGET_ARCH_VARIANT)"; \
	echo "export TARGET_CPU_VARIANT=$(TARGET_CPU_VARIANT)"; \
	echo "export PRODUCT_MANUFACTURER=$(PRODUCT_MANUFACTURER)"; \
	echo "export MOZ_DISABLE_LTO=$(MOZ_DISABLE_LTO)"; \
	echo "export HOST_OS=$(HOST_OS)";	\
	echo "export PRODUCTION_OS_NAME=$(PRODUCTION_OS_NAME)"; \
	) > .var.profile
	unset CC_WRAPPER && unset CXX_WRAPPER && \
	export GECKO_OBJDIR="$(abspath $(GECKO_OBJDIR))" && \
	export GONK_PATH="$(abspath .)" && \
	export GONK_PRODUCT_NAME="$(TARGET_DEVICE)" && \
	export PLATFORM_VERSION="$(PLATFORM_SDK_VERSION)" && \
	export TARGET_ARCH="$(TARGET_ARCH)" && \
	export TARGET_ARCH_VARIANT="$(TARGET_ARCH_VARIANT)" && \
	export TARGET_CPU_VARIANT="$(TARGET_CPU_VARIANT)" && \
	export PRODUCT_MANUFACTURER="$(PRODUCT_MANUFACTURER)" && \
	export MOZ_DISABLE_LTO="$(MOZ_DISABLE_LTO)" && \
	export MOZ_SANDBOX_GPU_NODE="$(MOZ_SANDBOX_GPU_NODE)" && \
	export HOST_OS="$(HOST_OS)" && \
	export PRODUCTION_OS_NAME="$(PRODUCTION_OS_NAME)" && \
	(cd $(GECKO_PATH) ; $(SHELL) build-b2g.sh) && \
	(cd $(GECKO_PATH) ; $(SHELL) build-b2g.sh package) && \
	mkdir -p $(@D) && cp $(GECKO_OBJDIR)/dist/b2g-*.tar.bz2 $@
endif

.PHONY: buildsymbols
buildsymbols:
	$(MAKE) -C $(GECKO_OBJDIR) $@ TARGET_OUT_UNSTRIPPED="$(abspath $(TARGET_OUT_UNSTRIPPED))"

# Include a copy of the repo manifest that has the revisions used
ifneq ($(DISABLE_SOURCES_XML),true)
ifneq (,$(realpath .repo/manifest.xml))
REPO := $(shell which repo)
ifeq (,$(REPO))
REPO := ./.repo/repo/repo
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := sources.xml
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := DATA
LOCAL_MODULE_PATH  := $(TARGET_OUT)

include $(BUILD_SYSTEM)/base_rules.mk

# Use the native comamnd provided by git-repo to create sources.xml.
# The basic attributes that will be added into all depend on .repo/manifest.xml.
$(LOCAL_BUILT_MODULE):
	mkdir -p $(@D)
	$(REPO) manifest -r -o $@

# this dependency ensure the above rule will be executed if module is built
$(LOCAL_INSTALLED_MODULE) : $(LOCAL_BUILT_MODULE)
	$(copy-file-to-new-target)
endif
endif
