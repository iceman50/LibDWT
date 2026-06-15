ROOT := ../..

SHELL := cmd
.SHELLFLAGS := /c

ARCH ?= x64
CONFIG ?= release

PREFIX_x86 ?= i686-w64-mingw32-
PREFIX_x64 ?= x86_64-w64-mingw32-

ifeq ($(ARCH),x86)
TOOLPREFIX := $(PREFIX_x86)
else ifeq ($(ARCH),x64)
TOOLPREFIX := $(PREFIX_x64)
else
$(error ARCH must be x86 or x64)
endif

ifeq ($(CONFIG),debug)
CFG_DEFINES := -D_DEBUG
CFG_CXXFLAGS := -O2 -g
else ifeq ($(CONFIG),release)
CFG_DEFINES := -DNDEBUG
CFG_CXXFLAGS := -O2
else
$(error CONFIG must be debug or release)
endif

CXX := $(TOOLPREFIX)g++
AR := ar
RANLIB := ranlib
RC := windres

BUILD_DIR := build/$(ARCH)/$(CONFIG)
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := $(BUILD_DIR)/lib
BIN_DIR := $(BUILD_DIR)/bin

DWT_LIB := $(LIB_DIR)/libdwt.a
MULTI_EXE := $(BIN_DIR)/MultiControlExample.exe
RICH_EXE := $(BIN_DIR)/RichTextCapabilities.exe
CUSTOM_EXE := $(BIN_DIR)/CustomDrawExample.exe
TEST_EXE := $(BIN_DIR)/FrameworkTests.exe
MANIFEST_RC := common-controls-v6.rc
MANIFEST_OBJ := $(OBJ_DIR)/projects/mingw-w64/common-controls-v6.res.o
MULTI_ICON_RC := $(ROOT)/examples/MultiControlExample/resources.rc
MULTI_ICON_OBJ := $(OBJ_DIR)/examples/MultiControlExample/resources.res.o

INCLUDES := -I$(ROOT)/dwt/include
COMMON_DEFINES := \
	-DUNICODE \
	-D_UNICODE \
	-DWINVER=0x0601 \
	-D_WIN32_WINNT=0x0601 \
	-D_WIN32_IE=0x0A00 \
	-DNOMINMAX \
	-D_CRT_SECURE_NO_WARNINGS

CPPFLAGS := $(INCLUDES) $(COMMON_DEFINES) $(CFG_DEFINES)
CXXFLAGS := -std=gnu++17 -Wall -Wextra -Wno-unknown-pragmas -Wno-missing-field-initializers -Wno-unused-parameter -Wno-strict-aliasing $(CFG_CXXFLAGS)
LDFLAGS := -mwindows -Wl,--major-subsystem-version,6,--minor-subsystem-version,1
LDLIBS := \
	-lcomctl32 \
	-ldwmapi \
	-lgdi32 \
	-lshlwapi \
	-lole32 \
	-loleaut32 \
	-luuid \
	-lshell32 \
	-lcomdlg32 \
	-ladvapi32 \
	-luxtheme

DWT_SOURCES := \
	$(wildcard $(ROOT)/dwt/src/*.cpp) \
	$(wildcard $(ROOT)/dwt/src/util/*.cpp) \
	$(wildcard $(ROOT)/dwt/src/util/win32/*.cpp) \
	$(wildcard $(ROOT)/dwt/src/widgets/*.cpp)

MULTI_SOURCE := $(ROOT)/examples/MultiControlExample/main.cpp
RICH_SOURCE := $(ROOT)/examples/RichTextCapabilities/main.cpp
CUSTOM_SOURCE := $(ROOT)/examples/CustomDrawExample/main.cpp
TEST_SOURCE := $(ROOT)/tests/FrameworkTests/main.cpp

DWT_OBJS := $(patsubst $(ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(DWT_SOURCES))
MULTI_OBJ := $(patsubst $(ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(MULTI_SOURCE))
RICH_OBJ := $(patsubst $(ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(RICH_SOURCE))
CUSTOM_OBJ := $(patsubst $(ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(CUSTOM_SOURCE))
TEST_OBJ := $(patsubst $(ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SOURCE))

.PHONY: all build test x86-debug x86-release x64-debug x64-release clean print-config

all: x86-debug x86-release x64-debug x64-release

build: $(DWT_LIB) $(MULTI_EXE) $(RICH_EXE) $(CUSTOM_EXE) $(TEST_EXE)

test: $(TEST_EXE)
	.\$(subst /,\,$(TEST_EXE))

x86-debug:
	$(MAKE) build ARCH=x86 CONFIG=debug

x86-release:
	$(MAKE) build ARCH=x86 CONFIG=release

x64-debug:
	$(MAKE) build ARCH=x64 CONFIG=debug

x64-release:
	$(MAKE) build ARCH=x64 CONFIG=release

print-config:
	@echo ARCH=$(ARCH)
	@echo CONFIG=$(CONFIG)
	@echo CXX=$(CXX)
	@echo BUILD_DIR=$(BUILD_DIR)

$(DWT_LIB): $(DWT_OBJS)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(MULTI_EXE): $(MULTI_OBJ) $(MANIFEST_OBJ) $(MULTI_ICON_OBJ) $(DWT_LIB)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(LDFLAGS) -o $@ $(MULTI_OBJ) $(MANIFEST_OBJ) $(MULTI_ICON_OBJ) $(DWT_LIB) $(LDLIBS)

$(RICH_EXE): $(RICH_OBJ) $(MANIFEST_OBJ) $(DWT_LIB)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(LDFLAGS) -o $@ $(RICH_OBJ) $(MANIFEST_OBJ) $(DWT_LIB) $(LDLIBS)

$(CUSTOM_EXE): $(CUSTOM_OBJ) $(MANIFEST_OBJ) $(DWT_LIB)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(LDFLAGS) -o $@ $(CUSTOM_OBJ) $(MANIFEST_OBJ) $(DWT_LIB) $(LDLIBS)

$(TEST_EXE): $(TEST_OBJ) $(DWT_LIB)
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) -Wl,--major-subsystem-version,6,--minor-subsystem-version,1 -o $@ $(TEST_OBJ) $(DWT_LIB) $(LDLIBS)

$(MANIFEST_OBJ): $(MANIFEST_RC) ../windows-common.manifest
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(RC) -O coff -i $(MANIFEST_RC) -o $@

$(MULTI_ICON_OBJ): $(MULTI_ICON_RC) $(ROOT)/examples/MultiControlExample/resources/tray.ico $(ROOT)/examples/MultiControlExample/resource.h
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(RC) -O coff -i $(MULTI_ICON_RC) -o $@

$(OBJ_DIR)/%.o: $(ROOT)/%.cpp
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@if exist build rmdir /s /q build
