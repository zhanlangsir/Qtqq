TARGET = jsoncpp
TEMPLATE = lib
DESTDIR = ../../libs

CONFIG += staticlib warn_off
CONFIG -= gui

include (../../common.pri)

PROJECT_LIBS += $$LIBJSONCPP_NAME

include (jsoncpp.pri)
