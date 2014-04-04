# This file is generated by gyp; do not edit.

TOOLSET := target
TARGET := opus
DEFS_Debug := \
	'-D_FILE_OFFSET_BITS=64' \
	'-DUSE_LINUX_BREAKPAD' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_PEPPER_THREADING' \
	'-DENABLE_GPU=1' \
	'-DUSE_OPENSSL=1' \
	'-DENABLE_EGLIMAGE=1' \
	'-DUSE_SKIA=1' \
	'-DOPUS_BUILD' \
	'-DOPUS_EXPORT=' \
	'-DHAVE_LRINT' \
	'-DHAVE_LRINTF' \
	'-DVAR_ARRAYS' \
	'-DFIXED_POINT' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DHAVE_SYS_UIO_H' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG'

# Flags passed to all source files.
CFLAGS_Debug := -I$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi/include \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-format \
	-mthumb \
	-march=armv7-a \
	-mtune=cortex-a8 \
	-mfloat-abi=softfp \
	-mfpu=vfpv3-d16 \
	-fno-tree-sra \
	-fuse-ld=gold \
	-Wno-psabi \
	-mthumb-interwork \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	--sysroot=$(ANDROID_NDK_PATH)/platforms/android-9/arch-arm \
	-I. \
	-I$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/include \
	-Os \
	-g \
	-fomit-frame-pointer \
	-fdata-sections \
	-ffunction-sections

# Flags passed to only C files.
CFLAGS_C_Debug :=

# Flags passed to only C++ files.
CFLAGS_CC_Debug := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated \
	-Wno-abi

INCS_Debug := \
	-Ithird_party/opus/src/celt \
	-Ithird_party/opus/src/include \
	-Ithird_party/opus/src/silk \
	-Ithird_party/opus/src/silk/fixed

DEFS_Release := \
	'-D_FILE_OFFSET_BITS=64' \
	'-DUSE_LINUX_BREAKPAD' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_PEPPER_THREADING' \
	'-DENABLE_GPU=1' \
	'-DUSE_OPENSSL=1' \
	'-DENABLE_EGLIMAGE=1' \
	'-DUSE_SKIA=1' \
	'-DOPUS_BUILD' \
	'-DOPUS_EXPORT=' \
	'-DHAVE_LRINT' \
	'-DHAVE_LRINTF' \
	'-DVAR_ARRAYS' \
	'-DFIXED_POINT' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DHAVE_SYS_UIO_H' \
	'-DNDEBUG' \
	'-DNVALGRIND' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=0'

# Flags passed to all source files.
CFLAGS_Release := -I$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi/include \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-format \
	-mthumb \
	-march=armv7-a \
	-mtune=cortex-a8 \
	-mfloat-abi=softfp \
	-mfpu=vfpv3-d16 \
	-fno-tree-sra \
	-fuse-ld=gold \
	-Wno-psabi \
	-mthumb-interwork \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	--sysroot=$(ANDROID_NDK_PATH)/platforms/android-9/arch-arm \
	-I. \
	-I$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/include \
	-Os \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer

# Flags passed to only C files.
CFLAGS_C_Release :=

# Flags passed to only C++ files.
CFLAGS_CC_Release := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated \
	-Wno-abi

INCS_Release := \
	-Ithird_party/opus/src/celt \
	-Ithird_party/opus/src/include \
	-Ithird_party/opus/src/silk \
	-Ithird_party/opus/src/silk/fixed

OBJS := \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/bands.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/celt.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/celt_lpc.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/cwrs.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/entcode.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/entdec.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/entenc.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/kiss_fft.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/laplace.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/mathops.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/mdct.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/modes.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/pitch.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/quant_bands.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/rate.o \
	$(obj).target/$(TARGET)/third_party/opus/src/celt/vq.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/A2NLSF.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/ana_filt_bank_1.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/biquad_alt.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/bwexpander.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/bwexpander_32.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/check_control_input.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/CNG.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/code_signs.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/control_audio_bandwidth.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/control_codec.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/control_SNR.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/debug.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/dec_API.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_core.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_frame.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_indices.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_parameters.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_pitch.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decode_pulses.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/decoder_set_fs.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/enc_API.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/encode_indices.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/encode_pulses.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/apply_sine_window_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/autocorr_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/burg_modified_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/corrMatrix_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/encode_frame_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/find_LPC_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/find_LTP_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/find_pitch_lags_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/find_pred_coefs_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/k2a_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/k2a_Q16_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/LTP_analysis_filter_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/LTP_scale_ctrl_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/noise_shape_analysis_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/pitch_analysis_core_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/prefilter_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/process_gains_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/regularize_correlations_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/residual_energy16_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/residual_energy_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/schur64_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/schur_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/solve_LS_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/vector_ops_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/fixed/warped_autocorrelation_FIX.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/gain_quant.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/HP_variable_cutoff.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/init_decoder.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/init_encoder.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/inner_prod_aligned.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/interpolate.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/lin2log.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/log2lin.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/LP_variable_cutoff.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/LPC_analysis_filter.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/LPC_inv_pred_gain.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF2A.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_decode.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_del_dec_quant.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_encode.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_stabilize.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_unpack.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_VQ.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NLSF_VQ_weights_laroia.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NSQ.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/NSQ_del_dec.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/pitch_est_tables.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/PLC.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/process_NLSFs.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/quant_LTP_gains.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_down2.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_down2_3.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_private_AR2.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_private_down_FIR.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_private_IIR_FIR.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_private_up2_HQ.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/resampler_rom.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/shell_coder.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/sigm_Q15.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/sort.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_decode_pred.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_encode_pred.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_find_predictor.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_LR_to_MS.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_MS_to_LR.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/stereo_quant_pred.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/sum_sqr_shift.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/table_LSF_cos.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_gain.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_LTP.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_NLSF_CB_NB_MB.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_NLSF_CB_WB.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_other.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_pitch_lag.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/tables_pulses_per_block.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/VAD.o \
	$(obj).target/$(TARGET)/third_party/opus/src/silk/VQ_WMat_EC.o \
	$(obj).target/$(TARGET)/third_party/opus/src/src/opus.o \
	$(obj).target/$(TARGET)/third_party/opus/src/src/opus_decoder.o \
	$(obj).target/$(TARGET)/third_party/opus/src/src/opus_encoder.o \
	$(obj).target/$(TARGET)/third_party/opus/src/src/opus_multistream.o \
	$(obj).target/$(TARGET)/third_party/opus/src/src/repacketizer.o

# Add to the list of files we specially track dependencies for.
all_deps += $(OBJS)

# CFLAGS et al overrides must be target-local.
# See "Target-specific Variable Values" in the GNU Make manual.
$(OBJS): TOOLSET := $(TOOLSET)
$(OBJS): GYP_CFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_C_$(BUILDTYPE))
$(OBJS): GYP_CXXFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_CC_$(BUILDTYPE))

# Suffix rules, putting all outputs into $(obj).

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# Try building from generated source, too.

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# End of this set of suffix rules
### Rules for final target.
LDFLAGS_Debug := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,-z,noexecstack \
	-fPIC \
	-Wl,-z,relro \
	-Wl,-z,now \
	-fuse-ld=gold \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	--sysroot=$(ANDROID_NDK_PATH)/platforms/android-9/arch-arm \
	-Wl,--icf=safe \
	-L. \
	-L$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi \
	-Wl,-O1 \
	-Wl,--as-needed \
	-Wl,--gc-sections

LDFLAGS_Release := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,-z,noexecstack \
	-fPIC \
	-Wl,-z,relro \
	-Wl,-z,now \
	-fuse-ld=gold \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	--sysroot=$(ANDROID_NDK_PATH)/platforms/android-9/arch-arm \
	-Wl,--icf=safe \
	-L. \
	-L$(ANDROID_NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.4.3/libs/armeabi \
	-Wl,-O1 \
	-Wl,--as-needed \
	-Wl,--gc-sections

LIBS := \
	 \
	-lgnustl_static \
	$(ANDROID_NDK_PATH)toolchains/arm-linux-androideabi-4.4.3/prebuilt/$(PLATFORM)/bin/../lib/gcc/arm-linux-androideabi/4.4.3/libgcc.a \
	-lc \
	-ldl \
	-lstdc++ \
	-lm

$(obj).target/third_party/opus/libopus.a: GYP_LDFLAGS := $(LDFLAGS_$(BUILDTYPE))
$(obj).target/third_party/opus/libopus.a: LIBS := $(LIBS)
$(obj).target/third_party/opus/libopus.a: TOOLSET := $(TOOLSET)
$(obj).target/third_party/opus/libopus.a: $(OBJS) FORCE_DO_CMD
	$(call do_cmd,alink)

all_deps += $(obj).target/third_party/opus/libopus.a
# Add target alias
.PHONY: opus
opus: $(obj).target/third_party/opus/libopus.a

# Add target alias to "all" target.
.PHONY: all
all: opus

# Add target alias
.PHONY: opus
opus: $(builddir)/libopus.a

# Copy this to the static library output path.
$(builddir)/libopus.a: TOOLSET := $(TOOLSET)
$(builddir)/libopus.a: $(obj).target/third_party/opus/libopus.a FORCE_DO_CMD
	$(call do_cmd,copy)

all_deps += $(builddir)/libopus.a
# Short alias for building this static library.
.PHONY: libopus.a
libopus.a: $(obj).target/third_party/opus/libopus.a $(builddir)/libopus.a

# Add static library to "all" target.
.PHONY: all
all: $(builddir)/libopus.a
