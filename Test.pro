QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#pro for msvc utf8 code
msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8
msvc:QMAKE_CXXFLAGS += -source-charset:utf-8
msvc:QMAKE_CXXFLAGS_WARN_ON += -wd4819
#cpp #pragma execution_character_set("utf-8")

debug: {
DESTDIR = $$PWD/debug
} else {
DESTDIR = $$PWD/release
}

LIBS += -L$$DESTDIR -lQSettingsUtf8

message($$DESTDIR)

INCLUDEPATH += $$PWD

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ui/hiscombobox.cpp

HEADERS += \
    mainwindow.h \
    ui/hiscombobox.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
