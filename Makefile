BUILD_DIR := build
ASSETS_DIR := assets
FILESYSTEM_DIR := filesystem
T3D_INST := tiny3d

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

SRC_DIRS := src cosmoslib64
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

TARGET := deltarune64
TARGET_STR := "Deltarune 64"
ROM := $(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
DFS := $(BUILD_DIR)/$(TARGET).dfs

IMAGE_LIST = $(shell find assets/ -type f -name '*.png')
FONT_LIST  = $(shell find assets/ -type f -name '*.ttf')
MODEL_LIST  = $(shell find assets/ -type f -name '*.glb')
SOUND_LIST  = $(shell find assets/ -type f -name '*.wav')
SOUND2_LIST = $(shell find assets/ -type f -name '*.mp3')
MUSIC_LIST  = $(shell find assets/ -type f -name '*.xm')
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(IMAGE_LIST:%.png=%.sprite))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(FONT_LIST:%.ttf=%.font64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MODEL_LIST:%.glb=%.t3dm))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(SOUND_LIST:%.wav=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(SOUND2_LIST:%.mp3=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MUSIC_LIST:%.xm=%.xm64))

final: $(ROM)
$(ROM): N64_ROM_TITLE=$(TARGET_STR)
$(ROM): $(DFS) 
$(DFS): $(ASSETS_LIST)
$(ELF): $(O_FILES)

$(FILESYSTEM_DIR)/%.sprite: $(ASSETS_DIR)/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.font64: $(ASSETS_DIR)/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.t3dm: $(ASSETS_DIR)/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) $(T3DM_FLAGS) "$<" $@
	$(N64_BINDIR)/mkasset -c 2 -o $(dir $@) $@

AUDIOCONV_FLAGS = --wav-resample 32000 --wav-mono

$(FILESYSTEM_DIR)/%.wav64: $(ASSETS_DIR)/%.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.wav64: $(ASSETS_DIR)/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.xm64: $(ASSETS_DIR)/%.xm
	@mkdir -p $(dir $@)
	@echo "    [XM] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

clean:
	rm -rf $(ROM) $(BUILD_DIR) filesystem

-include $(wildcard $(BUILD_DIR)/*.d)