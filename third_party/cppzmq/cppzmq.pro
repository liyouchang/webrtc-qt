#-------------------------------------------------
#
# Project created by QtCreator 2014-03-11T13:18:23
#
#-------------------------------------------------

QT       -= core gui

TARGET = cppzmq
TEMPLATE = lib
CONFIG += staticlib



include( ../third_party.pri)

INCLUDEPATH   +=    include



win32 {

DEFINES += ZMQ_HAVE_WINDOWS ZMQ_STATIC  __WINDOWS__


}

macx {
    DEFINES +=  HAVE_FORK  HAVE_INTTYPES_H HAVE_NETINET_IN_H \
            HAVE_STDINT_H HAVE_STDLIB_H HAVE_STRING_H HAVE_SYS_TYPES_H \
            ZMQ_HAVE_IFADDRS ZMQ_HAVE_UIO  ZMQ_HAVE_TCP_KEEPALIVE

    DEFINES += ZMQ_HAVE_OSX

    LIBS += -lpthread
}
ios {
    DEFINES +=  HAVE_FORK  HAVE_INTTYPES_H HAVE_NETINET_IN_H \
            HAVE_STDINT_H HAVE_STDLIB_H HAVE_STRING_H HAVE_SYS_TYPES_H \
            ZMQ_HAVE_IFADDRS ZMQ_HAVE_UIO  ZMQ_HAVE_TCP_KEEPALIVE

    DEFINES += ZMQ_HAVE_OSX
#ZMQ_HAVE_TCP_KEEPCNT ZMQ_HAVE_TCP_KEEPINTVL

}
linux {
    DEFINES += HAVE_CLOCK_GETTIME HAVE_FORK HAVE_INTTYPES_H HAVE_LIBRT \
            HAVE_NETINET_IN_H HAVE_STDINT_H HAVE_STDLIB_H HAVE_STRING_H \
            HAVE_SYS_EVENTFD_H HAVE_SYS_TYPES_H ZMQ_HAVE_IFADDRS \
            ZMQ_HAVE_SOCK_CLOEXEC ZMQ_HAVE_TCP_KEEPCNT ZMQ_HAVE_TCP_KEEPIDLE \
            ZMQ_HAVE_TCP_KEEPINTVL ZMQ_HAVE_UIO

    DEFINES += ZMQ_HAVE_LINUX
}
android {

    DEFINES -= ZMQ_HAVE_SOCK_CLOEXEC ZMQ_HAVE_IFADDRS
    DEFINES +=ZMQ_HAVE_ANDROID
}

#message($$DEFINES)

SOURCES += \
    src/address.cpp \
    src/clock.cpp \
    src/ctx.cpp \
    src/curve_client.cpp \
    src/curve_server.cpp \
    src/dealer.cpp \
    src/devpoll.cpp \
    src/dist.cpp \
    src/epoll.cpp \
    src/err.cpp \
    src/fq.cpp \
    src/io_object.cpp \
    src/io_thread.cpp \
    src/ip.cpp \
    src/ipc_address.cpp \
    src/ipc_connecter.cpp \
    src/ipc_listener.cpp \
    src/kqueue.cpp \
    src/lb.cpp \
    src/mailbox.cpp \
    src/mechanism.cpp \
    src/msg.cpp \
    src/mtrie.cpp \
    src/null_mechanism.cpp \
    src/object.cpp \
    src/options.cpp \
    src/own.cpp \
    src/pair.cpp \
    src/pgm_receiver.cpp \
    src/pgm_sender.cpp \
    src/pgm_socket.cpp \
    src/pipe.cpp \
    src/plain_mechanism.cpp \
    src/poll.cpp \
    src/poller_base.cpp \
    src/precompiled.cpp \
    src/proxy.cpp \
    src/pub.cpp \
    src/pull.cpp \
    src/push.cpp \
    src/random.cpp \
    src/raw_decoder.cpp \
    src/raw_encoder.cpp \
    src/reaper.cpp \
    src/rep.cpp \
    src/req.cpp \
    src/router.cpp \
    src/select.cpp \
    src/session_base.cpp \
    src/signaler.cpp \
    src/socket_base.cpp \
    src/stream_engine.cpp \
    src/stream.cpp \
    src/sub.cpp \
    src/tcp_address.cpp \
    src/tcp_connecter.cpp \
    src/tcp_listener.cpp \
    src/tcp.cpp \
    src/thread.cpp \
    src/trie.cpp \
    src/v1_decoder.cpp \
    src/v1_encoder.cpp \
    src/v2_decoder.cpp \
    src/v2_encoder.cpp \
    src/xpub.cpp \
    src/xsub.cpp \
    src/zmq_utils.cpp \
    src/zmq.cpp

HEADERS += \
    src/address.hpp \
    src/array.hpp \
    src/atomic_counter.hpp \
    src/atomic_ptr.hpp \
    src/blob.hpp \
    src/clock.hpp \
    src/command.hpp \
    src/config.hpp \
    src/ctx.hpp \
    src/curve_client.hpp \
    src/curve_server.hpp \
    src/dbuffer.hpp \
    src/dealer.hpp \
    src/decoder.hpp \
    src/devpoll.hpp \
    src/dist.hpp \
    src/encoder.hpp \
    src/epoll.hpp \
    src/err.hpp \
    src/fd.hpp \
    src/fq.hpp \
    src/i_decoder.hpp \
    src/i_encoder.hpp \
    src/i_engine.hpp \
    src/i_poll_events.hpp \
    src/io_object.hpp \
    src/io_thread.hpp \
    src/ip.hpp \
    src/ipc_address.hpp \
    src/ipc_connecter.hpp \
    src/ipc_listener.hpp \
    src/kqueue.hpp \
    src/lb.hpp \
    src/likely.hpp \
    src/mailbox.hpp \
    src/mechanism.hpp \
    src/msg.hpp \
    src/mtrie.hpp \
    src/mutex.hpp \
    src/null_mechanism.hpp \
    src/object.hpp \
    src/options.hpp \
    src/own.hpp \
    src/pair.hpp \
    src/pgm_receiver.hpp \
    src/pgm_sender.hpp \
    src/pgm_socket.hpp \
    src/pipe.hpp \
    src/plain_mechanism.hpp \
    src/platform.hpp \
    src/poll.hpp \
    src/poller_base.hpp \
    src/poller.hpp \
    src/precompiled.hpp \
    src/proxy.hpp \
    src/pub.hpp \
    src/pull.hpp \
    src/push.hpp \
    src/random.hpp \
    src/raw_decoder.hpp \
    src/raw_encoder.hpp \
    src/reaper.hpp \
    src/rep.hpp \
    src/req.hpp \
    src/router.hpp \
    src/select.hpp \
    src/session_base.hpp \
    src/signaler.hpp \
    src/socket_base.hpp \
    src/stdint.hpp \
    src/stream_engine.hpp \
    src/stream.hpp \
    src/sub.hpp \
    src/tcp_address.hpp \
    src/tcp_connecter.hpp \
    src/tcp_listener.hpp \
    src/tcp.hpp \
    src/thread.hpp \
    src/trie.hpp \
    src/v1_decoder.hpp \
    src/v1_encoder.hpp \
    src/v2_decoder.hpp \
    src/v2_encoder.hpp \
    src/v2_protocol.hpp \
    src/windows.hpp \
    src/wire.hpp \
    src/xpub.hpp \
    src/xsub.hpp \
    src/ypipe_base.hpp \
    src/ypipe_conflate.hpp \
    src/ypipe.hpp \
    src/yqueue.hpp \
    include/zmq.h \
    include/zmq_utils.h \
    include/zmq.hpp \
    include/zhelpers.hpp \
    include/zmsg.hpp

