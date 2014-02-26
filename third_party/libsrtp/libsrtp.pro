#-------------------------------------------------
#
# Project created by QtCreator 2014-02-21T11:31:12
#
#-------------------------------------------------

QT       -= core gui

TARGET = srtp
TEMPLATE = lib
CONFIG += staticlib

include( ../third_party.pri)


INCLUDEPATH     += \
        ./config \
      'srtp/include' \
      'srtp/crypto/include' \


DEFINES +=  HAVE_STDLIB_H HAVE_STRING_H
DEFINES += CPU_CISC

win32 {

DEFINES += INLINE=__inline \
           HAVE_BYTESWAP_METHODS_H \
           SIZEOF_UNSIGNED_LONG=4 \
           SIZEOF_UNSIGNED_LONG_LONG=8

} else {

DEFINES += \
    HAVE_INT16_T \
    HAVE_INT32_T \
    HAVE_INT8_T \
    HAVE_UINT16_T \
    HAVE_UINT32_T \
    HAVE_UINT64_T \
    HAVE_UINT8_T \
    HAVE_STDINT_H \
    HAVE_INTTYPES_H \
    HAVE_NETINET_IN_H \
    INLINE=inline

}


HEADERS += \
    srtp/include/ekt.h \
    srtp/include/getopt_s.h \
    srtp/include/rtp_priv.h \
    srtp/include/rtp.h \
    srtp/include/srtp_priv.h \
    srtp/include/srtp.h \
    srtp/include/ut_sim.h \
    srtp/crypto/include/aes_cbc.h \
    srtp/crypto/include/aes_icm.h \
    srtp/crypto/include/aes.h \
    srtp/crypto/include/alloc.h \
    srtp/crypto/include/auth.h \
    srtp/crypto/include/cipher.h \
    srtp/crypto/include/crypto_kernel.h \
    srtp/crypto/include/crypto_math.h \
    srtp/crypto/include/crypto_types.h \
    srtp/crypto/include/crypto.h \
    srtp/crypto/include/cryptoalg.h \
    srtp/crypto/include/datatypes.h \
    srtp/crypto/include/err.h \
    srtp/crypto/include/gf2_8.h \
    srtp/crypto/include/hmac.h \
    srtp/crypto/include/integers.h \
    srtp/crypto/include/kernel_compat.h \
    srtp/crypto/include/key.h \
    srtp/crypto/include/null_auth.h \
    srtp/crypto/include/null_cipher.h \
    srtp/crypto/include/prng.h \
    srtp/crypto/include/rand_source.h \
    srtp/crypto/include/rdb.h \
    srtp/crypto/include/rdbx.h \
    srtp/crypto/include/sha1.h \
    srtp/crypto/include/stat.h \
    srtp/crypto/include/xfm.h

SOURCES += \
    srtp/srtp/ekt.c \
    srtp/srtp/srtp.c \
    srtp/crypto/cipher/aes_cbc.c \
    srtp/crypto/cipher/aes_icm.c \
    srtp/crypto/cipher/aes.c \
    srtp/crypto/cipher/cipher.c \
    srtp/crypto/cipher/null_cipher.c \
    srtp/crypto/hash/auth.c \
    srtp/crypto/hash/hmac.c \
    srtp/crypto/hash/null_auth.c \
    srtp/crypto/hash/sha1.c \
    srtp/crypto/kernel/alloc.c \
    srtp/crypto/kernel/crypto_kernel.c \
    srtp/crypto/kernel/err.c \
    srtp/crypto/kernel/key.c \
    srtp/crypto/math/datatypes.c \
    srtp/crypto/math/gf2_8.c \
    srtp/crypto/math/stat.c \
    srtp/crypto/replay/rdb.c \
    srtp/crypto/replay/rdbx.c \
    srtp/crypto/replay/ut_sim.c \
    srtp/crypto/rng/ctr_prng.c \
    srtp/crypto/rng/prng.c \
    srtp/crypto/rng/rand_source.c




