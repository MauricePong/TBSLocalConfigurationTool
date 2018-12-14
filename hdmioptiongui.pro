#-------------------------------------------------
#
# Project created by QtCreator 2017-11-21T18:38:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = TBSLocalConfigurationTool
TEMPLATE    = app
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj
DESTDIR     = $$PWD/../bin

SOURCES     += main.cpp \
    tbsfunc.cpp \
    threadtbs.cpp \
    dsenum.cpp \
	Rtp.cpp \
	Udp.cpp \
	Socket.cpp\
	SourceTsFile.cpp
	
SOURCES     += iconhelper.cpp
SOURCES     += appinit.cpp
SOURCES     += hdmioptiongui.cpp

HEADERS     += iconhelper.h \
    tbsfunc.h \
    threadtbs.h \
    dsenum.h \
    Version.h \
	VBufferT.h \
	VBuffer.h \
	Udp.h \
	Rtp.h \
	TLock.h \
	Socket.h \
	string_t.h \
	SourceTsFile.h
HEADERS     += appinit.h
HEADERS     += hdmioptiongui.h

FORMS       += hdmioptiongui.ui

RESOURCES   += main.qrc
RESOURCES   += qss.qrc
CONFIG      += qt warn_off
INCLUDEPATH += $$PWD \
$$PWD/DirectShow/Lib/x86/include 
#INCLUDEPATH += "D:/Program Files/Microsoft Visual Studio/dshow/Samples/C++/DirectShow/BaseClasses"
#$$PWD/DirectShow/Lib/x86/include 			
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lamstrmid
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -ldmoguids
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lksproxy
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lksuser
LIBS        += -L$$PWD/DirectShow/Lib/x86/  -lmsdmo
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lquartz
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lstrmiids
#LIBS        += "-LD:/Program Files/Microsoft Visual Studio/dshow/Samples/C++/DirectShow/BaseClasses/Release" -lSTRMBASE
LIBS        += -L$$PWD/DirectShow/Lib/x86/ -lSTRMBASE

#LIBS        +=-lpthread -ldl $$PWD/libpci.so
RC_FILE = hdmioptiongui.rc
#DEFINES += QT_NO_WARNING_OUTPUT\
 #                QT_NO_DEBUG_OUTPUT


