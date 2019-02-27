#-------------------------------------------------
#
# Project created by QtCreator 2018-08-05T10:57:59
#
#-------------------------------------------------

QT       += core gui network
RC_ICONS = appicon.ico
#RC_FILE += version.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# PRECOMPILED_HEADER = stable.h # 使用预编译头加快编译速度
# QMAKE_CXXFLAGS += /MP         # 使用并行编译

TARGET = WriterFly
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += dirlayout\
    dirlayout/dirsettings\
    editor\
    mainwindow\
    settings\
    utils\
    widgets\
    widgets/animations\
    widgets/events\
    widgets/transitions\
    widgets/titlebar\
    im_ex_port\
    threads\
    globals\
    network


SOURCES += main.cpp\
        mainwindow/mainwindow.cpp \
    widgets/titlebar/customtitlebar.cpp \
    widgets/titlebar/customtitlebarwindow.cpp \
    widgets/titlebar/mytitlebar.cpp \
    mainwindow/basewindow.cpp \
    editor/noveleditor.cpp \
    dirlayout/chapterinfo.cpp \
    dirlayout/noveldiritem.cpp \
    dirlayout/dirsettings/noveldirsettings.cpp \
    globals/globalvar.cpp \
    dirlayout/noveldirmdata.cpp \
    dirlayout/noveldirgroup.cpp \
    dirlayout/noveldirlistview.cpp \
    utils/stringutil.cpp \
    editor/novelai.cpp \
    editor/noveleditinginfo.cpp \
    utils/fileutil.cpp \
    editor/novelaibase.cpp \
    editor/stackwidget.cpp \
    widgets/transitions/stackwidgetanimation.cpp \
    settings/usettings.cpp \
    editor/editorrecorderitem.cpp \
    utils/frisoutil.cpp \
    widgets/events/splitterwidget.cpp \
    widgets/events/dragsizewidget.cpp \
    settings/usersettingswindow.cpp \
    widgets/animations/aniswitch.cpp \
    settings/usersettingsitem.cpp \
    widgets/animations/roundlabel.cpp \
    widgets/animations/numberlabel.cpp \
    widgets/titlebar/anivlabel.cpp \
    widgets/animations/anifbutton.cpp \
    widgets/animations/aninlabel.cpp \
    mainwindow/transparentcontainer.cpp \
    utils/lexicon.cpp \
    widgets/transitions/zoomgeometryanimationwidget.cpp \
    mainwindow/novelschedulewidget.cpp \
    widgets/events/mylabel.cpp \
    im_ex_port/im_ex_window.cpp \
    im_ex_port/exportpage.cpp \
    im_ex_port/importpage.cpp \
    im_ex_port/importmzfypage.cpp \
    dirlayout/dirsettings/dirsettingswidget.cpp \
    globals/runtimeinfo.cpp \
    editor/selectionitemlist.cpp \
    network/filedownloadmanager.cpp \
    editor/editorcursor.cpp \
    editor/editorinputmanager.cpp \
    editor/editorinputcharactor.cpp

HEADERS  += mainwindow/mainwindow.h \
    widgets/titlebar/customtitlebar.h \
    widgets/titlebar/customtitlebarwindow.h \
    widgets/titlebar/mytitlebar.h \
    mainwindow/basewindow.h \
    editor/noveleditor.h \
    globals/defines.h \
    dirlayout/chapterinfo.h \
    dirlayout/noveldirmodel.h \
    dirlayout/noveldiritem.h \
    dirlayout/dirsettings/noveldirsettings.h \
    globals/globalvar.h \
    dirlayout/noveldirmdata.h \
    dirlayout/noveldirgroup.h \
    dirlayout/noveldirlistview.h \
    settings/settings.h \
    utils/stringutil.h \
    editor/novelai.h \
    editor/noveleditinginfo.h \
    settings/usettings.h \
    utils/fileutil.h \
    dirlayout/noveldirdelegate.h \
    editor/novelaibase.h \
    editor/stackwidget.h \
    widgets/transitions/stackwidgetanimation.h \
    editor/editorrecorderitem.h \
    utils/frisoutil.h \
    utils/pinyinutil.h \
    widgets/events/splitterwidget.h \
    widgets/events/dragsizewidget.h \
    settings/usersettingswindow.h \
    settings/usersettingsitem.h \
    widgets/animations/roundlabel.h \
    widgets/animations/numberlabel.h \
    widgets/titlebar/anivlabel.h \
    widgets/animations/aniswitch.h \
    widgets/animations/anifbutton.h \
    widgets/animations/aninlabel.h \
    mainwindow/transparentcontainer.h \
    threads/frisothread.h \
    utils/lexicon.h \
    threads/lexiconsthread.h \
    globals/runtimeinfo.h \
    widgets/transitions/zoomgeometryanimationwidget.h \
    mainwindow/novelschedulewidget.h \
    widgets/events/mylabel.h \
    im_ex_port/im_ex_window.h \
    im_ex_port/exportpage.h \
    im_ex_port/importpage.h \
    im_ex_port/importmzfypage.h \
    dirlayout/dirsettings/dirsettingswidget.h \
    globals/stable.h \
    editor/selectionitem.h \
    editor/selectionitemlist.h \
    threads/applicationupdatethread.h \
    network/ \
    network/filedownloadmanager.h \
    utils/netutil.h \
    editor/editorcursor.h \
    editor/editorinputmanager.h \
    editor/editorinputcharactor.h

DISTFILES += \
    image/close.png \
    image/max.png \
    image/min.png \
    image/sure.png \
    image/appicon.png \
    stylesheet/CustomTitleBar.css \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    docs/README.md \
    docs/UPDATE.md

RESOURCES += \
    resources.qrc

#Qt5.6.1版的
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lFrisoLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lFrisoLibd
else:unix: #LIBS += -L$$PWD/./ -lFrisoLib

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

