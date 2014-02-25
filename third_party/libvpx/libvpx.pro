#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T16:15:52
#
#-------------------------------------------------

QT       -= core gui

TARGET = vpx
TEMPLATE = lib
CONFIG += staticlib

CONFIG -= qt

DESTDIR = $$PWD/../../libs

OBJECTS_DIR = $$PWD/../../tmp/$$TARGET


linux {
    QMAKE_CFLAGS += -mmmx -msse2 -mssse3

    INCLUDEPATH += $$PWD/source/config/linux/x64
}

INCLUDEPATH += $$PWD/source/libvpx  $$PWD/source/config


ASM_SOURCES += \
    source/libvpx/third_party/x86inc/x86inc.asm \
    source/libvpx/vp9/common/x86/vp9_copy_sse2.asm \
    source/libvpx/vp9/common/x86/vp9_intrapred_sse2.asm \
    source/libvpx/vp9/common/x86/vp9_intrapred_ssse3.asm \
    source/libvpx/vp9/common/x86/vp9_loopfilter_mmx.asm \
    source/libvpx/vp9/common/x86/vp9_postproc_mmx.asm \
    source/libvpx/vp9/common/x86/vp9_postproc_sse2.asm \
    source/libvpx/vp9/common/x86/vp9_subpixel_8t_sse2.asm \
    source/libvpx/vp9/common/x86/vp9_subpixel_8t_ssse3.asm \
    source/libvpx/vpx_ports/emms.asm \
    source/libvpx/vpx_ports/x86_abi_support.asm \
    source/libvpx/vp8/common/x86/subpixel_ssse3.asm \
    source/libvpx/vp8/common/x86/subpixel_sse2.asm

YASM_INCLUDES = $$join(INCLUDEPATH,"-i","-i")
yasmasm.output = ${QMAKE_FILE_BASE}.o
yasmasm.commands = yasm -Xvc -f elf -o ${QMAKE_FILE_BASE}.o -g cv8 -rnasm -pnasm ${QMAKE_FILE_NAME}
yasmasm.input = ASM_SOURCES


SOURCES += \
    source/libvpx/vp9/decoder/vp9_decodeframe.c \
    source/libvpx/vp9/decoder/vp9_decodemv.c \
    source/libvpx/vp9/decoder/vp9_detokenize.c \
    source/libvpx/vp9/decoder/vp9_dsubexp.c \
    source/libvpx/vp9/decoder/vp9_onyxd_if.c \
    source/libvpx/vp9/decoder/vp9_reader.c \
    source/libvpx/vp9/decoder/vp9_thread.c \
    source/libvpx/vp8/common/alloccommon.c \
    source/libvpx/vp8/common/blockd.c \
    source/libvpx/vp8/common/context.c \
    source/libvpx/vp8/common/debugmodes.c \
    source/libvpx/vp8/common/dequantize.c \
    source/libvpx/vp8/common/entropy.c \
    source/libvpx/vp8/common/entropymode.c \
    source/libvpx/vp8/common/entropymv.c \
    source/libvpx/vp8/common/extend.c \
    source/libvpx/vp8/common/filter.c \
    source/libvpx/vp8/common/findnearmv.c \
    source/libvpx/vp8/common/idct_blk.c \
    source/libvpx/vp8/common/idctllm.c \
    source/libvpx/vp8/common/loopfilter_filters.c \
    source/libvpx/vp8/common/loopfilter.c \
    source/libvpx/vp8/common/mbpitch.c \
    source/libvpx/vp8/common/mfqe.c \
    source/libvpx/vp8/common/modecont.c \
    source/libvpx/vp8/common/postproc.c \
    source/libvpx/vp8/common/quant_common.c \
    source/libvpx/vp8/common/reconinter.c \
    source/libvpx/vp8/common/reconintra.c \
    source/libvpx/vp8/common/reconintra4x4.c \
    source/libvpx/vp8/common/rtcd.c \
    source/libvpx/vp8/common/sad_c.c \
    source/libvpx/vp8/common/setupintrarecon.c \
    source/libvpx/vp8/common/swapyv12buffer.c \
    source/libvpx/vp8/common/textblit.c \
    source/libvpx/vp8/common/treecoder.c \
    source/libvpx/vp8/common/variance_c.c \
    source/libvpx/vp8/decoder/dboolhuff.c \
    source/libvpx/vp8/decoder/decodeframe.c \
    source/libvpx/vp8/decoder/decodemv.c \
    source/libvpx/vp8/decoder/detokenize.c \
    source/libvpx/vp8/decoder/onyxd_if.c \
    source/libvpx/vp8/decoder/threading.c \
    source/libvpx/vp8/encoder/bitstream.c \
    source/libvpx/vp8/encoder/boolhuff.c \
    source/libvpx/vp8/encoder/dct.c \
    source/libvpx/vp8/encoder/denoising.c \
    source/libvpx/vp8/encoder/encodeframe.c \
    source/libvpx/vp8/encoder/encodeintra.c \
    source/libvpx/vp8/encoder/encodemb.c \
    source/libvpx/vp8/encoder/encodemv.c \
    source/libvpx/vp8/encoder/ethreading.c \
    source/libvpx/vp8/encoder/firstpass.c \
    source/libvpx/vp8/encoder/lookahead.c \
    source/libvpx/vp8/encoder/mcomp.c \
    source/libvpx/vp8/encoder/modecosts.c \
    source/libvpx/vp8/encoder/mr_dissim.c \
    source/libvpx/vp8/encoder/onyx_if.c \
    source/libvpx/vp8/encoder/pickinter.c \
    source/libvpx/vp8/encoder/picklpf.c \
    source/libvpx/vp8/encoder/psnr.c \
    source/libvpx/vp8/encoder/quantize.c \
    source/libvpx/vp8/encoder/ratectrl.c \
    source/libvpx/vp8/encoder/rdopt.c \
    source/libvpx/vp8/encoder/segmentation.c \
    source/libvpx/vp8/encoder/ssim.c \
    source/libvpx/vp8/encoder/temporal_filter.c \
    source/libvpx/vp8/encoder/tokenize.c \
    source/libvpx/vp8/encoder/treewriter.c \
    source/libvpx/vp8/encoder/vp8_asm_enc_offsets.c \
    source/libvpx/vp8/vp8_dx_iface.c \
    source/libvpx/vp8/vp8_cx_iface.c \
    source/libvpx/vp9/common/generic/vp9_systemdependent.c \
    source/libvpx/vp9/common/vp9_alloccommon.c \
    source/libvpx/vp9/common/vp9_common_data.c \
    source/libvpx/vp9/common/vp9_convolve.c \
    source/libvpx/vp9/common/vp9_debugmodes.c \
    source/libvpx/vp9/common/vp9_entropy.c \
    source/libvpx/vp9/common/vp9_entropymode.c \
    source/libvpx/vp9/common/vp9_entropymv.c \
    source/libvpx/vp9/common/vp9_filter.c \
    source/libvpx/vp9/common/vp9_idct.c \
    source/libvpx/vp9/common/vp9_loopfilter_filters.c \
    source/libvpx/vp9/common/vp9_loopfilter.c \
    source/libvpx/vp9/common/vp9_mvref_common.c \
    source/libvpx/vp9/common/vp9_pred_common.c \
    source/libvpx/vp9/common/vp9_prob.c \
    source/libvpx/vp9/common/vp9_quant_common.c \
    source/libvpx/vp9/common/vp9_reconinter.c \
    source/libvpx/vp9/common/vp9_reconintra.c \
    source/libvpx/vp9/common/vp9_rtcd.c \
    source/libvpx/vp9/common/vp9_scale.c \
    source/libvpx/vp9/common/vp9_scan.c \
    source/libvpx/vp9/common/vp9_seg_common.c \
    source/libvpx/vp9/common/vp9_textblit.c \
    source/libvpx/vp9/common/vp9_tile_common.c \
    source/libvpx/vp9/common/x86/vp9_asm_stubs.c \
    source/libvpx/vp9/common/x86/vp9_idct_intrin_sse2.c \
    source/libvpx/vp9/common/x86/vp9_loopfilter_intrin_sse2.c \
    source/libvpx/vp9/vp9_cx_iface.c \
    source/libvpx/vp9/vp9_dx_iface.c \
    source/libvpx/vpx_mem/vpx_mem.c \
    source/libvpx/vpx_ports/arm_cpudetect.c \
    source/libvpx/vpx_ports/x86_cpuid.c \
    source/libvpx/vpx_scale/generic/gen_scalers.c \
    source/libvpx/vpx_scale/generic/vpx_scale.c \
    source/libvpx/vpx_scale/generic/yv12config.c \
    source/libvpx/vpx_scale/generic/yv12extend.c \
    source/libvpx/vpx_scale/vpx_scale_rtcd.c \
    source/libvpx/vpx/src/vpx_codec.c \
    source/libvpx/vpx/src/vpx_decoder.c \
    source/libvpx/vpx/src/vpx_encoder.c \
    source/libvpx/vpx/src/vpx_image.c \
    source/libvpx/vp8/common/x86/idct_blk_mmx.c \
    source/libvpx/vp8/common/x86/variance_mmx.c \
    source/libvpx/vp8/encoder/x86/vp8_enc_stubs_mmx.c \
    source/libvpx/vp8/encoder/x86/vp8_enc_stubs_sse2.c \
    source/libvpx/vp8/common/x86/variance_ssse3.c \
    source/libvpx/vp8/common/x86/variance_sse2.c \
    source/libvpx/vp8/common/x86/idct_blk_sse2.c \
    source/libvpx/vp8/common/x86/recon_wrapper_sse2.c \
    source/libvpx/vp8/encoder/x86/quantize_sse2.c \
    source/libvpx/vp8/encoder/x86/denoising_sse2.c \
    source/libvpx/vp8/common/generic/systemdependent.c \
    source/libvpx/vp8/common/x86/vp8_asm_stubs.c \






HEADERS += \
    source/libvpx/vp9/decoder/vp9_decodeframe.h \
    source/libvpx/vp9/decoder/vp9_decodemv.h \
    source/libvpx/vp9/decoder/vp9_detokenize.h \
    source/libvpx/vp9/decoder/vp9_dsubexp.h \
    source/libvpx/vp9/decoder/vp9_onyxd_int.h \
    source/libvpx/vp9/decoder/vp9_onyxd.h \
    source/libvpx/vp9/decoder/vp9_read_bit_buffer.h \
    source/libvpx/vp9/decoder/vp9_reader.h \
    source/libvpx/vp9/decoder/vp9_thread.h \
    source/libvpx/vp8/common/alloccommon.h \
    source/libvpx/vp8/common/blockd.h \
    source/libvpx/vp8/common/coefupdateprobs.h \
    source/libvpx/vp8/common/common.h \
    source/libvpx/vp8/common/default_coef_probs.h \
    source/libvpx/vp8/common/entropy.h \
    source/libvpx/vp8/common/entropymode.h \
    source/libvpx/vp8/common/entropymv.h \
    source/libvpx/vp8/common/extend.h \
    source/libvpx/vp8/common/filter.h \
    source/libvpx/vp8/common/findnearmv.h \
    source/libvpx/vp8/common/header.h \
    source/libvpx/vp8/common/invtrans.h \
    source/libvpx/vp8/common/loopfilter.h \
    source/libvpx/vp8/common/modecont.h \
    source/libvpx/vp8/common/mv.h \
    source/libvpx/vp8/common/onyx.h \
    source/libvpx/vp8/common/onyxc_int.h \
    source/libvpx/vp8/common/onyxd.h \
    source/libvpx/vp8/common/postproc.h \
    source/libvpx/vp8/common/ppflags.h \
    source/libvpx/vp8/common/pragmas.h \
    source/libvpx/vp8/common/quant_common.h \
    source/libvpx/vp8/common/reconinter.h \
    source/libvpx/vp8/common/reconintra4x4.h \
    source/libvpx/vp8/common/setupintrarecon.h \
    source/libvpx/vp8/common/swapyv12buffer.h \
    source/libvpx/vp8/common/systemdependent.h \
    source/libvpx/vp8/common/threading.h \
    source/libvpx/vp8/common/treecoder.h \
    source/libvpx/vp8/common/variance.h \
    source/libvpx/vp8/common/vp8_entropymodedata.h \
    source/libvpx/vp8/decoder/dboolhuff.h \
    source/libvpx/vp8/decoder/decodemv.h \
    source/libvpx/vp8/decoder/decoderthreading.h \
    source/libvpx/vp8/decoder/detokenize.h \
    source/libvpx/vp8/decoder/ec_types.h \
    source/libvpx/vp8/decoder/error_concealment.h \
    source/libvpx/vp8/decoder/onyxd_int.h \
    source/libvpx/vp8/decoder/treereader.h \
    source/libvpx/vp8/encoder/bitstream.h \
    source/libvpx/vp8/encoder/block.h \
    source/libvpx/vp8/encoder/boolhuff.h \
    source/libvpx/vp8/encoder/dct_value_cost.h \
    source/libvpx/vp8/encoder/dct_value_tokens.h \
    source/libvpx/vp8/encoder/defaultcoefcounts.h \
    source/libvpx/vp8/encoder/denoising.h \
    source/libvpx/vp8/encoder/encodeframe.h \
    source/libvpx/vp8/encoder/encodeintra.h \
    source/libvpx/vp8/encoder/encodemb.h \
    source/libvpx/vp8/encoder/encodemv.h \
    source/libvpx/vp8/encoder/firstpass.h \
    source/libvpx/vp8/encoder/lookahead.h \
    source/libvpx/vp8/encoder/mcomp.h \
    source/libvpx/vp8/encoder/modecosts.h \
    source/libvpx/vp8/encoder/mr_dissim.h \
    source/libvpx/vp8/encoder/onyx_int.h \
    source/libvpx/vp8/encoder/pickinter.h \
    source/libvpx/vp8/encoder/psnr.h \
    source/libvpx/vp8/encoder/quantize.h \
    source/libvpx/vp8/encoder/ratectrl.h \
    source/libvpx/vp8/encoder/rdopt.h \
    source/libvpx/vp8/encoder/segmentation.h \
    source/libvpx/vp8/encoder/tokenize.h \
    source/libvpx/vp8/encoder/treewriter.h \
    source/libvpx/vp9/common/vp9_alloccommon.h \
    source/libvpx/vp9/common/vp9_blockd.h \
    source/libvpx/vp9/common/vp9_common_data.h \
    source/libvpx/vp9/common/vp9_common.h \
    source/libvpx/vp9/common/vp9_convolve.h \
    source/libvpx/vp9/common/vp9_entropy.h \
    source/libvpx/vp9/common/vp9_entropymode.h \
    source/libvpx/vp9/common/vp9_entropymv.h \
    source/libvpx/vp9/common/vp9_enums.h \
    source/libvpx/vp9/common/vp9_filter.h \
    source/libvpx/vp9/common/vp9_idct.h \
    source/libvpx/vp9/common/vp9_loopfilter.h \
    source/libvpx/vp9/common/vp9_mv.h \
    source/libvpx/vp9/common/vp9_mvref_common.h \
    source/libvpx/vp9/common/vp9_onyx.h \
    source/libvpx/vp9/common/vp9_onyxc_int.h \
    source/libvpx/vp9/common/vp9_postproc.h \
    source/libvpx/vp9/common/vp9_ppflags.h \
    source/libvpx/vp9/common/vp9_pragmas.h \
    source/libvpx/vp9/common/vp9_pred_common.h \
    source/libvpx/vp9/common/vp9_prob.h \
    source/libvpx/vp9/common/vp9_quant_common.h \
    source/libvpx/vp9/common/vp9_reconinter.h \
    source/libvpx/vp9/common/vp9_reconintra.h \
    source/libvpx/vp9/common/vp9_scale.h \
    source/libvpx/vp9/common/vp9_scan.h \
    source/libvpx/vp9/common/vp9_seg_common.h \
    source/libvpx/vp9/common/vp9_systemdependent.h \
    source/libvpx/vp9/common/vp9_textblit.h \
    source/libvpx/vp9/common/vp9_tile_common.h \
    source/libvpx/vp9/common/x86/vp9_postproc_x86.h \
    source/libvpx/vp9/vp9_iface_common.h \
    source/libvpx/vpx_mem/vpx_mem.h \
    source/libvpx/vpx_ports/arm.h \
    source/libvpx/vpx_ports/asm_offsets.h \
    source/libvpx/vpx_ports/config.h \
    source/libvpx/vpx_ports/emmintrin_compat.h \
    source/libvpx/vpx_ports/mem_ops_aligned.h \
    source/libvpx/vpx_ports/mem_ops.h \
    source/libvpx/vpx_ports/mem.h \
    source/libvpx/vpx_ports/vpx_once.h \
    source/libvpx/vpx_ports/vpx_timer.h \
    source/libvpx/vpx_ports/x86.h \
    source/libvpx/vpx_scale/vpx_scale.h

OTHER_FILES += \
    libvpx.gyp \
    libvpx_srcs_x86_64.gypi \
    copy_obj.py \
    ../yasm/yasm_compile.gypi \
    libvpx_srcs_x86_64_intrinsics.gypi \
    $$ASM_SOURCES
