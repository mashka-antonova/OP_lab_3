QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    draw.cpp \
    entrypoint.c \
    iterator.c \
    list.c \
    load_file_data.c \
    logic.c \
    main.cpp \
    mainwindow.cpp \
    metrix.c \
    parser.c

HEADERS += \
    appcontext.h \
    demography.h \
    draw.h \
    entrypoint.h \
    iterator.h \
    list.h \
    load_file_data.h \
    logic.h \
    mainwindow.h \
    metrix.h \
    parser.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    source.qrc
