
INCLUDEPATH     += $$PWD
DEPENDPATH      += $$PWD


SOURCES += $$PWD/VideoWall.cpp \
    $$PWD/PlayWidget.cpp \
    $$PWD/AVService.cpp

HEADERS  += $$PWD/VideoWall.h \
    $$PWD/PlayWidget.h \
    $$PWD/AVService.h \
    $$PWD/AVPlay.h

win32 {
  #lht:need to build with msvc,there will be some error in mingw
        LIBS += -L$$PWD
        LIBS += -lAVPlay
} else:mac {
} else {
}


