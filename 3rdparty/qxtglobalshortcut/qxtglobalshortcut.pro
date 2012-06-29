TARGET = $$LIBQXTGLOBALSHORTCUT_NAME
TEMPLATE = lib
DESTDIR = ../../libs

CONFIG += staticlib warn_off
CONFIG -= gui

include(qxtglobalshortcut.pri)
