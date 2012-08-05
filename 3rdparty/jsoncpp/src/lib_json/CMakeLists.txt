
set(jsoncpp_SOURCES
	json_internalarray.inl
	json_internalmap.inl
	json_reader.cpp
	json_value.cpp
	json_valueiterator.inl
	json_writer.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${JSONCPP_INCLUDE_DIR})

# 使用jsonxx防止跟系统已有的冲突
add_library(jsoncxx SHARED
	${jsoncpp_SOURCES}
)

set_target_properties(jsoncxx
	PROPERTIES VERSION 0.6.0
	SOVERSION 0)
install(TARGETS jsoncxx LIBRARY DESTINATION ${libdir})
