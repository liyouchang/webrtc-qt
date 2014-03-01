TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += .. ../talk ../third_party ../third_party/webrtc ../webrtc \
            ../third_party/wtl/include ../third_party/expat/files/lib \
            ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include

Debug:DESTDIR = ../Debug
Release:DESTDIR = ../Release


win32 {

DEFINES += ANGLE_DX11 EXPAT_RELATIVE_PATH FEATURE_ENABLE_VOICEMAIL \
           GTEST_RELATIVE_PATH JSONCPP_RELATIVE_PATH LOGGING=1\
           SRTP_RELATIVE_PATH FEATURE_ENABLE_SSL FEATURE_ENABLE_PSTN HAVE_SRTP\
           HAVE_WEBRTC_VIDEO HAVE_WEBRTC_VOICE USE_WEBRTC_DEV_BRANCH \
           _WIN32_WINNT=0x0602 WINVER=0x0602 WIN32 _WINDOWS NOMINMAX PSAPI_VERSION=1 \
           _CRT_RAND_S CERT_CHAIN_PARA_HAS_EXTRA_FIELDS WIN32_LEAN_AND_MEAN \
           _ATL_NO_OPENGL _HAS_EXCEPTIONS=0 _SECURE_ATL V8_DEPRECATION_WARNINGS \
           CHROMIUM_BUILD TOOLKIT_VIEWS=1 USE_AURA=1 USE_ASH=1 USE_DEFAULT_RENDER_THEME=1 \
           USE_LIBJPEG_TURBO=1 ENABLE_ONE_CLICK_SIGNIN ENABLE_REMOTING=1 ENABLE_WEBRTC=1 \
           ENABLE_PEPPER_CDMS ENABLE_CONFIGURATION_POLICY ENABLE_INPUT_SPEECH \
           ENABLE_NOTIFICATIONS ENABLE_EGLIMAGE=1 __STD_C _CRT_SECURE_NO_DEPRECATE \
           _SCL_SECURE_NO_DEPRECATE NTDDI_VERSION=0x06020000 _USING_V110_SDK71_ \
           ENABLE_TASK_MANAGER=1 ENABLE_EXTENSIONS=1 ENABLE_PLUGIN_INSTALLATION=1 \
           ENABLE_PLUGINS=1 ENABLE_SESSION_SERVICE=1 ENABLE_THEMES=1 ENABLE_AUTOFILL_DIALOG=1 \
           ENABLE_BACKGROUND=1 ENABLE_AUTOMATION=1 ENABLE_GOOGLE_NOW=1 CLD_VERSION=2 \
           ENABLE_FULL_PRINTING=1 ENABLE_PRINTING=1 ENABLE_SPELLCHECK=1 \
           ENABLE_CAPTIVE_PORTAL_DETECTION=1 ENABLE_APP_LIST=1 ENABLE_SETTINGS_APP=1 \
           ENABLE_MANAGED_USERS=1 ENABLE_MDNS=1 LIBPEERCONNECTION_LIB=1 \

Debug:DEFINES += _DEBUG XML_STATIC _CRT_NONSTDC_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE \
            DYNAMIC_ANNOTATIONS_ENABLED=1 WTF_USE_DYNAMIC_ANNOTATIONS=1

Release:DEFINES += XML_STATIC _CRT_NONSTDC_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE   \
           NDEBUG NVALGRIND DYNAMIC_ANNOTATIONS_ENABLED=0

LIBS += -lwininet -ldnsapi -lversion -lmsimg32 -lws2_32  -lusp10 -lpsapi \
        -ldbghelp  -lwinmm -lshlwapi -lkernel32 -lgdi32 -lwinspool -lcomdlg32 \
        -ladvapi32 -lshell32 -lole32 -loleaut32 -luser32 -luuid -lodbc32 \
        -lodbccp32 -ldelayimp -lcrypt32 -liphlpapi -lsecur32

} else:mac {
} else {
}

LIBS += -L$$DESTDIR -L$$DESTDIR/lib \
        -llibjingle -lexpat -ljsoncpp -lcrnss -lnss_static \
        -lcrnspr -lsqlite3 -licui18n -licuuc

SOURCES += \
    ../talk/examples/peerconnection/server/data_socket.cc \
    ../talk/examples/peerconnection/server/main.cc \
    ../talk/examples/peerconnection/server/peer_channel.cc \
    ../talk/examples/peerconnection/server/utils.cc

HEADERS += \
    ../talk/examples/peerconnection/server/data_socket.h \
    ../talk/examples/peerconnection/server/peer_channel.h \
    ../talk/examples/peerconnection/server/utils.h

