all: \
    $(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.h

$(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.h \
    : \
    $(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.o \
    $(BUILT_PRODUCTS_DIR)/../$(CONFIGURATION)/libvpx_obj_int_extract \
    ../../../third_party/libvpx/obj_int_extract.py
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets"
	@echo note: "Generate assembly offsets $(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.o"
	python ../../../third_party/libvpx/obj_int_extract.py -e "$(BUILT_PRODUCTS_DIR)/../$(CONFIGURATION)/libvpx_obj_int_extract" -f cheader -b "$(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.o" -o "$(SHARED_INTERMEDIATE_DIR)/audio_processing/asm_offsets/nsx_core_neon_offsets.h"
