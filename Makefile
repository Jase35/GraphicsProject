TARGET    := app
SRC_DIR   := src
INC_DIR   := include
LIB_DIR   := libs
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj

CXX := g++
CC  := gcc

MKDIR = mkdir -p "$1"
RM_RF = rm -rf "$1"

WARN     := -Wall -Wextra -Wpedantic
INC_DIRS := $(INC_DIR) \
            $(SRC_DIR) \
            $(LIB_DIR)/entt-main/single_include \
            $(LIB_DIR)/fastgltf-main/include \
            $(LIB_DIR)/stb-master \
            $(LIB_DIR)/imgui-master \
            $(LIB_DIR)/imgui-master/backends
CPPFLAGS := $(addprefix -I,$(INC_DIRS)) -MMD -MP -DGLFW_INCLUDE_NONE
CXXFLAGS := -std=c++17 $(WARN)
CFLAGS   := -std=c17 -Wall
LDFLAGS  :=

LDLIBS   := -lglfw3 -lopengl32 -lgdi32 -lsimdjson

BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE),release)
  OPT      := -O2 -DNDEBUG
else ifeq ($(BUILD_TYPE),debug)
  OPT := -g -O0
else
  $(error BUILD_TYPE must be 'debug' or 'release', got '$(BUILD_TYPE)')
endif

CXXFLAGS += $(OPT)
CFLAGS   += $(OPT)

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CXX_SRCS := $(call rwildcard,$(SRC_DIR),*.cpp)
C_SRCS   := $(call rwildcard,$(SRC_DIR),*.c)

LIB_SRCS := $(LIB_DIR)/fastgltf-main/src/base64.cpp \
            $(LIB_DIR)/fastgltf-main/src/fastgltf.cpp \
            $(LIB_DIR)/fastgltf-main/src/io.cpp \
            $(LIB_DIR)/imgui-master/imgui.cpp \
            $(LIB_DIR)/imgui-master/imgui_draw.cpp \
            $(LIB_DIR)/imgui-master/imgui_tables.cpp \
            $(LIB_DIR)/imgui-master/imgui_widgets.cpp \
            $(LIB_DIR)/imgui-master/imgui_demo.cpp \
            $(LIB_DIR)/imgui-master/backends/imgui_impl_glfw.cpp \
            $(LIB_DIR)/imgui-master/backends/imgui_impl_opengl3.cpp

OBJS := $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) \
        $(C_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
        $(LIB_SRCS:$(LIB_DIR)/%.cpp=$(OBJ_DIR)/libs/%.o)
DEPS := $(OBJS:.o=.d)

EXE := $(BUILD_DIR)/$(TARGET).exe

.PHONY: all run clean rebuild
.DEFAULT_GOAL := all

all: $(EXE)

$(EXE): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo "==> $@ ($(BUILD_TYPE))"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(call MKDIR,$(dir $@))
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/libs/%.o: $(LIB_DIR)/%.cpp | $(OBJ_DIR)
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CPPFLAGS) -std=c++17 $(OPT) -c $< -o $@

$(BUILD_DIR) $(OBJ_DIR):
	@$(call MKDIR,$@)

run: $(EXE)
	@cd $(BUILD_DIR) && ./$(TARGET).exe

clean:
	@$(call RM_RF,$(BUILD_DIR))
	@echo "==> cleaned"

rebuild: clean all

print-%:
	@echo "$* = $($*)"

-include $(DEPS)