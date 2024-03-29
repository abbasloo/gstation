#-------------------------------------------------
#
# Project created by QtCreator 2019-06-14T23:51:58
#
#-------------------------------------------------

QT       += core gui widgets printsupport serialport webengine webenginewidgets webchannel websockets

TARGET = gstation
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp \
    ImageWidget.cpp \
    NMEAParserLib/NMEASentenceRMC.cpp \
    NMEAParserLib/NMEASentenceGSV.cpp \
    NMEAParserLib/NMEASentenceGSA.cpp \
    NMEAParserLib/NMEASentenceGGA.cpp \
    NMEAParserLib/NMEASentenceBase.cpp \
    NMEAParserLib/NMEAParserPacket.cpp \
    NMEAParserLib/NMEAParser.cpp \
    websockettransport.cpp \
    websocketclientwrapper.cpp

HEADERS += \
        mainwindow.h \
    qcustomplot.h \
    ImageWidget.h \
    NMEAParserLib/NMEASentenceRMC.h \
    NMEAParserLib/NMEASentenceGSV.h \
    NMEAParserLib/NMEASentenceGSA.h \
    NMEAParserLib/NMEASentenceGGA.h \
    NMEAParserLib/NMEASentenceBase.h \
    NMEAParserLib/NMEAParserPacket.h \
    NMEAParserLib/NMEAParserData.h \
    NMEAParserLib/NMEAParser.h \
    websockettransport.h \
    websocketclientwrapper.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_shape -lopencv_videoio

RESOURCES += \
    resources.qrc

DISTFILES += \
    qwebchannel.js
