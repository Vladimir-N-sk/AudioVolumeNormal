TEMPLATE = app
TARGET = AudioVolumeNormal

QT += widgets core gui svg

QTPLUGIN.platforms = qminimal qxcb

HEADERS = window.h \
    audio.h \
    pbdialog.h \
    pbdialog3.h


SOURCES = window.cpp \
main.cpp \
audio.cpp \
    pbdialog.cpp \
    pbdialog3.cpp

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused -Wimplicit-fallthrough

FORMS += \
    pbdialog.ui \
    pbdialog3.ui

RESOURCES += \
    icon.qrc

