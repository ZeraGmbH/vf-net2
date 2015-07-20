#-------------------------------------------------
#
# Project created by QtCreator 2014-11-26T15:11:42
#
#-------------------------------------------------

TEMPLATE = lib

#dependencies
VEIN_DEP_EVENT = 1
VEIN_DEP_COMP = 1
VEIN_DEP_PROTOBUF = 1
VEIN_DEP_TCP = 1
VEIN_DEP_HELPER = 1

exists( ../../project-paths.pri ) {
  include(../../project-paths.pri)
}

QT -= gui
QT += core network

TARGET = vein-net

DEFINES += VEINNET_LIBRARY

SOURCES += \
    vn_tcpsystem.cpp \
    vn_networksystem.cpp \
    vn_protocolevent.cpp \
    vn_protocolwrapper.cpp \
    vn_introspectionsystem.cpp \
    vn_networkstatusevent.cpp

HEADERS +=\
        veinnet_global.h \
    vn_tcpsystem.h \
    vn_networksystem.h \
    vn_protocolevent.h \
    vn_protocolwrapper.h \
    vn_introspectionsystem.h \
    vn_networkstatusevent.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CXXFLAGS += -Wall -Wfloat-equal -std=c++0x
