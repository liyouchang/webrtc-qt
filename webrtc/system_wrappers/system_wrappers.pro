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
        'source/clock.cc'\
        'source/condition_variable.cc'\
        'source/cpu_info.cc'\
        'source/cpu_features.cc'\
        'source/critical_section.cc'\
        'source/data_log.cc'\
        'source/data_log_c.cc'\
        'source/data_log_no_op.cc'\
        'source/event.cc'\
        'source/event_tracer.cc'\
        'source/file_impl.cc'\
        'source/logging.cc'\
        'source/rw_lock.cc'\
        'source/rw_lock_generic.cc'\
        'source/sleep.cc'\
        'source/sort.cc'\
        'source/tick_util.cc'\
        'source/thread.cc'\
        'source/trace_impl.cc'\



linux {

DEFINES += WEBRTC_THREAD_RR

SOURCES += \
        'source/atomic32_posix.cc'\
        'source/rw_lock_posix.cc'\
        'source/trace_posix.cc'\
        'source/thread_posix.cc'\
        'source/event_posix.cc'\
        'source/critical_section_posix.cc'\
        'source/condition_variable_posix.cc'\

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

OTHER_FILES += \
    source/system_wrappers.gyp

HEADERS += \
    interface/aligned_malloc.h \
    interface/asm_defines.h \
    interface/atomic32.h \
    interface/clock.h \
    interface/compile_assert.h \
    interface/compile_assert_c.h \
    interface/condition_variable_wrapper.h \
    interface/constructor_magic.h \
    interface/cpu_features_wrapper.h \
    interface/cpu_info.h \
    interface/critical_section_wrapper.h \
    interface/data_log.h \
    interface/data_log_c.h \
    interface/data_log_impl.h \
    interface/event_tracer.h \
    interface/event_wrapper.h \
    interface/file_wrapper.h \
    interface/fix_interlocked_exchange_pointer_win.h \
    interface/logcat_trace_context.h \
    interface/logging.h \
    interface/ref_count.h \
    interface/rw_lock_wrapper.h \
    interface/scoped_ptr.h \
    interface/scoped_refptr.h \
    interface/sleep.h \
    interface/sort.h \
    interface/static_instance.h \
    interface/stringize_macros.h \
    interface/template_util.h \
    interface/thread_annotations.h \
    interface/thread_wrapper.h \
    interface/tick_util.h \
    interface/trace.h \
    interface/trace_event.h
