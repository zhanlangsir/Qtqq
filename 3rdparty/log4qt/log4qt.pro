TARGET = log4qt
TEMPLATE = lib
DESTDIR = ../../libs

CONFIG += staticlib warn_off
CONFIG -= gui

include(../../common.pri)

PROJECT_LIBS += $$LIBLOG4QT_NAME

include(src/log4qt/log4qt.pri)
