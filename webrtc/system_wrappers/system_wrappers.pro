#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T15:45:35
#
#-------------------------------------------------

QT       -= core gui

TARGET = system_wrappers
TEMPLATE = lib
CONFIG += staticlib


include (../webrtc_common.pri)

INCLUDEPATH += interface  source/spreadsortlib

#LIBS += -lrt



SOURCES += \
        'source/aligned_malloc.cc'\
        'source/atomic32_posix.cc'\
        'source/clock.cc'\
        'source/condition_variable.cc'\
        'source/condition_variable_posix.cc'\
        'source/cpu_info.cc'\
        'source/cpu_features.cc'\
        'source/critical_section.cc'\
        'source/critical_section_posix.cc'\
        'source/data_log.cc'\
        'source/data_log_c.cc'\
        'source/data_log_no_op.cc'\
        'source/event.cc'\
        'source/event_posix.cc'\
        'source/event_tracer.cc'\
        'source/file_impl.cc'\
        'source/logging.cc'\
        'source/rw_lock.cc'\
        'source/rw_lock_generic.cc'\
        'source/rw_lock_posix.cc'\
        'source/sleep.cc'\
        'source/sort.cc'\
        'source/tick_util.cc'\
        'source/thread.cc'\
        'source/thread_posix.cc'\
        'source/trace_impl.cc'\
        'source/trace_posix.cc'\



HEADERS +=



linux {

DEFINES += WEBRTC_THREAD_RR

SOURCES += \

}

macx {
SOURCES += \
        'source/atomic32_mac.cc'\

}

win32{
SOURCES += \
        'source/atomic32_win.cc'\
        'source/condition_variable_event_win.cc'\
        'source/condition_variable_native_win.cc'\
        'source/critical_section_win.cc'\
        'source/trace_win.cc'\
        'source/thread_win.cc'\
        'source/rw_lock_win.cc'\
        'source/event_win.cc'\

}

android {
SOURCES += \
        'source/logcat_trace_context.cc'\

}
