INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/include/autolink.h \
    $$PWD/include/config.h \
    $$PWD/include/json_features.h \
    $$PWD/include/forwards.h \
    $$PWD/include/json.h \
    $$PWD/include/reader.h \
    $$PWD/include/value.h \
    $$PWD/include/writer.h \
    $$PWD/include/json_tool.h \
    $$PWD/src/json_batchallocator.h
    
SOURCES += \
    $$PWD/src/json_internalarray.inl \
    $$PWD/src/json_internalmap.inl \
    $$PWD/src/json_reader.cpp \
    $$PWD/src/json_value.cpp \
    $$PWD/src/json_valueiterator.inl \
    $$PWD/src/json_writer.cpp 
