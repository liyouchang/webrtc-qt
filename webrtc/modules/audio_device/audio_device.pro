#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T10:01:34
#
#-------------------------------------------------

QT       -= core gui

TARGET = audio_device
TEMPLATE = lib
CONFIG += staticlib


include (../../webrtc_common.pri)



SOURCES += \
    dummy/audio_device_dummy.cc \
    dummy/audio_device_utility_dummy.cc \
    audio_device_buffer.cc \
    audio_device_generic.cc \
    audio_device_impl.cc \
    audio_device_utility.cc \


HEADERS += \
    dummy/audio_device_dummy.h \
    dummy/audio_device_utility_dummy.h \
    audio_device_buffer.h \
    audio_device_generic.h \
    audio_device_impl.h \
    audio_device_utility.h \
    audio_device_config.h \


linux {

INCLUDEPATH += linux

    DEFINES += LINUX_ALSA LINUX_PULSE

SOURCES += \
            'linux/alsasymboltable_linux.cc'\
            'linux/audio_device_alsa_linux.cc'\
            'linux/audio_device_utility_linux.cc'\
            'linux/audio_mixer_manager_alsa_linux.cc'\
            'linux/latebindingsymboltable_linux.cc'\

SOURCES += \
                    'linux/audio_device_pulse_linux.cc'\
                    'linux/audio_mixer_manager_pulse_linux.cc'\
                    'linux/pulseaudiosymboltable_linux.cc'\


}

win32 {

INCLUDEPATH += win


HEADERS += \
    win/audio_device_core_win.h \
    win/audio_device_utility_win.h \
    win/audio_device_wave_win.h \
    win/audio_mixer_manager_win.h


SOURCES += \
    win/audio_device_core_win.cc \
    win/audio_device_utility_win.cc \
    win/audio_device_wave_win.cc \
    win/audio_mixer_manager_win.cc



}
OTHER_FILES += \
    audio_device.gypi
