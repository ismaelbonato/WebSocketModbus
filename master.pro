QT += serialbus serialport widgets

TARGET = modbusmaster
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
        writeregistermodel.cpp

HEADERS  += mainwindow.h \
        writeregistermodel.h

FORMS    += mainwindow.ui

RESOURCES += \
    master.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/master
INSTALLS += target
