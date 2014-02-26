#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T15:01:47
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle_sound
TEMPLATE = lib
CONFIG += staticlib


include (talk_common.pri)

DESTDIR = $$output_dir/libs

HEADERS += \
        'sound/automaticallychosensoundsystem.h' \
        'sound/nullsoundsystem.h' \
        'sound/nullsoundsystemfactory.h'\
        'sound/platformsoundsystem.h' \
        'sound/platformsoundsystemfactory.h' \
        'sound/sounddevicelocator.h' \
        'sound/soundinputstreaminterface.h' \
        'sound/soundoutputstreaminterface.h' \
        'sound/soundsystemfactory.h' \
        'sound/soundsysteminterface.h' \
        'sound/soundsystemproxy.h'


SOURCES += \
        'sound/nullsoundsystem.cc'\
        'sound/nullsoundsystemfactory.cc'\
        'sound/platformsoundsystem.cc'\
        'sound/soundsysteminterface.cc' \
        'sound/soundsystemproxy.cc' \
        'sound/platformsoundsystemfactory.cc' \



unix:!macx{


HEADERS += \
            'sound/alsasoundsystem.h' \
            'sound/alsasymboltable.h' \
            'sound/linuxsoundsystem.h' \
            'sound/pulseaudiosoundsystem.h' \
            'sound/pulseaudiosymboltable.h'

SOURCES += \
            'sound/alsasoundsystem.cc' \
            'sound/alsasymboltable.cc' \
            'sound/linuxsoundsystem.cc' \
            'sound/pulseaudiosoundsystem.cc' \
            'sound/pulseaudiosymboltable.cc' \
}
