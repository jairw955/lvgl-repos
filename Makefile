.PHONY: all help menu menu8 menu9 FORCE

# 脚本目录
SCRIPT_DIR := script
BUILD_SCRIPT := $(SCRIPT_DIR)/build.sh

# 自动发现所有可用的 hook 文件（排除 build.sh）
HOOK_FILES := $(filter-out $(SCRIPT_DIR)/build.sh, $(wildcard $(SCRIPT_DIR)/*.sh))
HOOK_NAMES := $(foreach hook,$(HOOK_FILES),$(basename $(notdir $(hook))))

# 默认操作列表（可以根据需要扩展）
DEFAULT_ACTIONS := build configure

# 生成所有可能的 target 名称（用于 .PHONY 声明）
ALL_TARGETS := $(HOOK_NAMES) \
               $(foreach hook,$(HOOK_NAMES),$(foreach action,$(DEFAULT_ACTIONS),$(hook)-$(action)))

# 声明所有动态生成的 target 为 .PHONY
.PHONY: $(ALL_TARGETS)

# 为每个发现的 hook 动态生成规则
$(foreach hook,$(HOOK_NAMES),$(eval $(hook): FORCE ; @bash "$(BUILD_SCRIPT)" "$(hook)"))
$(foreach hook,$(HOOK_NAMES),$(foreach action,$(DEFAULT_ACTIONS),$(eval $(hook)-$(action): FORCE ; @bash "$(BUILD_SCRIPT)" "$(hook)-$(action)")))

# 默认目标
all: help

# 帮助信息
help:
	@echo "Available targets:"
	@echo ""
	@echo "Configuration:"
	@echo "  make menu          - Open menuconfig"
	@echo "  make menu8          - Open menuconfig for lvgl8"
	@echo "  make menu9          - Open menuconfig for lvgl9"
	@echo ""
	@echo "Build targets (auto-discovered from script/ directory):"
	@for hook in $(HOOK_NAMES); do \
		line="  $$hook"; \
		for action in $(DEFAULT_ACTIONS); do \
			line="$$line $$hook-$$action"; \
		done; \
		echo "$$line"; \
	done
	@echo ""
	@echo "Examples:"
	@echo "  make lvgl8              - Build lvgl8 (default action: build)"
	@echo "  make lvgl8-build        - Build lvgl8"
	@echo "  make lvgl8-configure    - Configure lvgl8"
	@echo "  make lvgl9-build        - Build lvgl9"
	@echo ""

# 配置菜单
menu:
	@bash "$(BUILD_SCRIPT)" menu

menu8:
	@bash "$(BUILD_SCRIPT)" menu8

menu9:
	@bash "$(BUILD_SCRIPT)" menu9


# FORCE 目标，确保依赖它的目标总是执行
FORCE:
