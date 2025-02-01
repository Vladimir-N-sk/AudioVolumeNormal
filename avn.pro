TEMPLATE = app
TARGET = AudioLevelNormal

QT += widgets core gui

QTPLUGIN.platforms = qminimal qxcb linuxfb

HEADERS = window.h \
    audio.h

SOURCES = window.cpp \
main.cpp \
audio.cpp

INCLUDEPATH += /media/sdb2/ffmpeg_new/include

LIBS += -L/media/sdb2/Qt/QtNew/lib
#LIBS += -L/media/sdb2/ffmpeg_new/lib
#LIBS += -lpostproc -lavformat  -lavcodec -lavutil -lavfilter -lswscale -lavdevice -lswresample -lx264 -lx265

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused

