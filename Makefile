LIBRARY := nvui
APPLICATION := uitest

LIBRARY_STATIC := lib$(LIBRARY).a

BUILD_DIR := build
SRC_DIR := src
SRC_SUBDIRS := core widgets core/platform

DEFINES := __USE_XOPEN _GNU_SOURCE NVEXPORT
INC_DIRS := include

LIBS := m glad2 triangulate stdc++
LIB_DIRS := 

CC := gcc

CCFLAGS := -Wall -std=c2x -Wstrict-prototypes -fPIC

LD := $(CC)
LDFLAGS := -shared

AR ?= ar
ARFLAGS := rcs

ifeq ($(CONFIG),release)
    DEFINES += NDEBUG
    CCFLAGS += -O2
    LDFLAGS += -s
else
    DEFINES += _DEBUG
    CCFLAGS += -g
endif

ifeq ($(OS),Windows_NT)
    LIBS += user32 gdi32 opengl32
    LIBRARY_NAME := $(LIBRARY).dll
    APPLICATION := $(APPLICATION).exe
    LIB_DIRS += $(LIB_GCC_PATH)
    INC_DIRS += $(INC_PATH)
    LDFLAGS += -mwindows 
else
    LIBRARY_NAME := lib$(LIBRARY).so
    LDFLAGS += -fvisibility=hidden
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS += GL
        DEFINES += 
    endif
    ifeq ($(UNAME_S),Darwin)
    endif
    ifeq ($(UNAME_S),FreeBSD)
    endif
endif

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -D,$(DEFINES)) -MMD -MP
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

# SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(wildcard $(SRC_DIR)/*.c) $(foreach pat,$(SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(pat)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

BUILD_DIRS := $(addprefix $(BUILD_DIR)/,$(SRC_SUBDIRS))

all: lib test
test: $(BUILD_DIR) $(APPLICATION)
lib: $(BUILD_DIR) $(LIBRARY_NAME)
lib_static: $(BUILD_DIR) $(LIBRARY_STATIC)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIRS)

$(APPLICATION): $(LIBRARY_NAME) test/main.c
	@echo "LD $@"
	@$(LD) -o $@ -l$(LIBRARY) -Iinclude -L. -mconsole -Wl,-rpath,. -std=c2x -g test/main.c

$(LIBRARY_STATIC): $(OBJS)
	@echo "AR $@"
	@$(AR) $(ARFLAGS) $@ $<

$(LIBRARY_NAME): $(OBJS)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIB_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@echo "CC $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

.PHONY: clean echo test lib lib_static
clean:
	@echo "RM $(BUILD_DIR)/"
	@rm -rf $(BUILD_DIR)
	@echo "RM $(APPLICATION)"
	@rm -f $(APPLICATION)
	@echo "RM $(LIBRARY_NAME)"
	@rm -f $(LIBRARY_NAME)
	@echo "RM $(LIBRARY_STATIC)"
	@rm -f $(LIBRARY_STATIC)

echo:
	@echo "LIBS= $(LIBS)"
	@echo "INC_DIRS= $(INC_DIRS)"
	@echo "CCFLAGS= $(CCFLAGS)"
	@echo "LDFLAGS= $(LDFLAGS)"

-include $(OBJS:.o=.d)
