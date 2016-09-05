QT += serialbus serialport widgets websockets

TARGET = modbusmaster
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
        writeregistermodel.cpp \
    webserver.cpp

HEADERS  += mainwindow.h \
        writeregistermodel.h \
    webserver.h

FORMS    += mainwindow.ui

RESOURCES += \
    master.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/master
INSTALLS += target

DISTFILES += \
    web/js/socketserver.js \
    web/index.html \
    LICENSE \
    README.md \
    doc/src/modbusmaster.qdoc \
    web/js/dygraph-combined.js
