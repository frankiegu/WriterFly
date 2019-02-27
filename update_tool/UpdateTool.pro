QT = core network
CONFIG += console
#CONFIG -= app_bundle

SOURCES += main.cpp \
    downloadmanager.cpp

HEADERS += \
    downloadmanager.h \
    fileutil.h

INCLUDEPATH += $$PWD/quazip/include

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/quazip/lib -lquazipd
} else {
    LIBS += -L$$PWD/quazip/lib -lquazip
}
