TEMPLATE = app
TARGET = compAlgo
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug | release) {
	CONFIGURATION = debug
} else {
	CONFIGURATION = release
}

OBJECTS_DIR = $$PWD/build/$$CONFIGURATION/obj
MOC_DIR = $$PWD/build/$$CONFIGURATION/moc
DESTDIR = $$PWD/build/$$CONFIGURATION/bin/

SOURCES += src/main.cpp \
    src/simulator.cpp \
    src/lteEnb/l2-mac.cpp \
    src/lteEnb/x2-channel.cpp \
    src/lteEnb/ff-mac-scheduler.cpp \
    src/helpers.cpp \
    src/lteEnb/ff-mac-sched-sap.cpp

HEADERS += \
    src/helpers.h \
    src/simulator.h \
    src/lteEnb/l2-mac.h \
    src/lteEnb/x2-channel.h \
    src/lteEnb/ff-mac-scheduler.h \
    src/lteEnb/ff-mac-sched-sap.h \
    src/messages.h


