TEMPLATE = app
TARGET = example

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

NS_BUILD_DIR = $$PWD/../3dparty/ns-3-allinone/ns-3.23/build
NS_HEADERS_DIR = $$PWD/../3dparty/ns-3-allinone/ns-3.23/build/ns3

LIBS += -L$$NS_BUILD_DIR
INCLUDEPATH += $$NS_BUILD_DIR
#INCLUDEPATH += $$NS_HEADERS_DIR

win32 {
    SHARED_LIB_FILES = $$files($$NS_BUILD_DIR/*.dll)
    for(FILE, SHARED_LIB_FILES) {
	BASENAME = $$basename(FILE)
	LIBS += -l$$replace(BASENAME,.dll,)
    }
}
unix {
    SHARED_LIB_FILES = $$files($$NS_BUILD_DIR/*.so)
    for(FILE, SHARED_LIB_FILES) {
	BASENAME = $$basename(FILE)
	CLEARNAME = $$replace(BASENAME,libns,ns)
	CLEARNAME = $$replace(CLEARNAME,.so,)
	LIBS += -l$$CLEARNAME
    }
}

CONFIG(debug, debug | release) {
	CONFIGURATION = debug
} else {
	CONFIGURATION = release
}

OBJECTS_DIR = $$PWD/build/$$CONFIGURATION/obj
MOC_DIR = $$PWD/build/$$CONFIGURATION/moc
DESTDIR = $$PWD/build/$$CONFIGURATION/bin/


DEFINES += NS3_LOG_ENABLE
DEFINES += NS3_ASSERT_ENABLE

SOURCES += \
    main.cpp

HEADERS += \
    helpers.h




