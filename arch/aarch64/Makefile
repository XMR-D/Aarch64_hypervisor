ARCH_DIR := arch/$(ARCH)
INCLUDES += -I$(ARCH_DIR)/include

.PHONY: config

#ORDER IS IMPORTANT ENTRY.S AT THE START
srcs-y += $(ARCH_DIR)/entry.S $(ARCH_DIR)/picolibc.c $(ARCH_DIR)/bootstrap.c $(ARCH_DIR)/hyp_mmu_setting.c $(ARCH_DIR)/tar.c $(ARCH_DIR)/bootstrap.S 