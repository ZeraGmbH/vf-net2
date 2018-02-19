#-------------------------------------------------
#
# Project created by QtCreator 2014-11-26T15:11:42
#
#-------------------------------------------------

TEMPLATE = lib


OTHER_FILES += ecs_schema.fbs
contains(DEFINES, BUILD_DEV_SST) {
  INCLUDEPATH += /work/downloads/git-clones/flatbuffers/include/
  QMAKE_CXXFLAGS += -isystem /work/downloads/git-clones/flatbuffers/include/
  FBUF_COMPILER = /work/downloads/git-clones/flatbuffers/flatc
}

#generate flatbuffers headers
!defined(FBUF_COMPILER, var) {
  message(using system flatbuffer compiler)
  FBUF_COMPILER = flatc
}

FBUF_SRCDIR=$$_PRO_FILE_PWD_
FBUF_IDL= $$FBUF_SRCDIR/ecs_schema.fbs
flatbuffers.target = ecs_schema_generated.h
flatbuffers.commands = $$FBUF_COMPILER --cpp -o $$FBUF_SRCDIR/ $$FBUF_IDL
flatbuffers.output = ecs_schema_generated.h
flatbuffers.CONFIG = target_predeps
QMAKE_EXTRA_TARGETS += flatbuffers
PRE_TARGETDEPS += ecs_schema_generated.h
QMAKE_CLEAN += ecs_schema_generated.h

# include autogenerated flatbuffer header
HEADERS += ecs_schema_generated.h


#dependencies
VEIN_DEP_EVENT = 1
VEIN_DEP_COMP = 1
VEIN_DEP_TCP2 = 1
VEIN_DEP_HELPER = 1

HEADERS +=\
        veinnet_global.h \
    vn_tcpsystem.h \
    vn_networksystem.h \
    vn_protocolevent.h \
    vn_introspectionsystem.h \
    vn_networkstatusevent.h

public_headers.files = $$HEADERS

exists( ../../vein-framework.pri ) {
  include(../../vein-framework.pri)
}

QT -= gui
QT += core network

TARGET = vein-net2

DEFINES += VEINNET_LIBRARY

SOURCES += \
    vn_tcpsystem.cpp \
    vn_networksystem.cpp \
    vn_protocolevent.cpp \
    vn_introspectionsystem.cpp \
    vn_networkstatusevent.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CXXFLAGS += -Wall -Wfloat-equal -std=c++0x
