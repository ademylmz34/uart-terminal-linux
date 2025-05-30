QT       += core gui
QT       += gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calibration_board.cpp \
    command_line.cpp \
    creator.cpp \
    log_parser.cpp \
    main.cpp \
    mainwindow.cpp \
    serial.cpp

HEADERS += \
    calibration_board.h \
    command_line.h \
    creator.h \
    enum_types.h \
    log_parser.h \
    mainwindow.h \
    serial.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
