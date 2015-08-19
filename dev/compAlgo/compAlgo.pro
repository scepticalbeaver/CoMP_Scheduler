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
    src/helpers.cpp \
    src/lteEnb/l2-mac.cpp \
    src/lteEnb/x2-channel.cpp \
    src/lteEnb/ff-mac-scheduler.cpp \
    src/lteEnb/ff-mac-sched-sap.cpp \
    src/lteEnb/trendIndicators/wma-indicator.cpp \
    src/lteEnb/trendIndicators/kama-indicator.cpp \
    src/lteEnb/trendIndicators/itrend-indicator.cpp \
    src/lteEnb/comp-decision-algo.cpp \
    src/lteEnb/trendIndicators/interpolation-indicator.cpp

HEADERS += \
    src/helpers.h \
    src/simulator.h \
    src/messages.h \
    src/lteEnb/l2-mac.h \
    src/lteEnb/x2-channel.h \
    src/lteEnb/ff-mac-scheduler.h \
    src/lteEnb/ff-mac-sched-sap.h \
    src/lteEnb/trendIndicators/wma-indicator.h \
    src/lteEnb/trendIndicators/kama-indicator.h \
    src/lteEnb/trendIndicators/itrend-indicator.h \
    src/lteEnb/comp-decision-algo.h \
    src/lteEnb/trendIndicators/interpolation-indicator.h


