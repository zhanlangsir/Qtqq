cmake_minimum_required(VERSION 2.6)

project(Qtqq)
set(version 0.8.0-alpha)

find_package(Qt4 REQUIRED)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

include(CheckIncludeFiles)
include(FindPkgConfig)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

if (NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/usr")
endif()

set(package	${PROJECT_NAME})
if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set(DEBUG_MODE 1)
    set(prefix ${PROJECT_SOURCE_DIR})
    set(pkgdatadir ${prefix}/data)
    set(PLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/plugins)
else ()
    set(prefix	${CMAKE_INSTALL_PREFIX})
    set(pkgdatadir ${prefix}/share/${package})
    set(PLUGIN_DIR ${libdir}/plugins)
endif ()

set(bindir ${prefix}/bin)
set(libdir ${prefix}/lib)
set(includedir ${prefix}/include)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(PLUGIN_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}/plugins)

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)

include_directories(${PROJECT_BINARY_DIR})

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(LOG4QT_INCLUDE_DIR ${THIRD_PARTY_DIR}/log4qt/src)
set(QXGS_INCLUDE_DIR ${THIRD_PARTY_DIR}/qxtglobalshortcut)
set(JSONCPP_INCLUDE_DIR ${THIRD_PARTY_DIR}/jsoncpp/include)
set(TRANSLATIONS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/translations)

add_subdirectory(cmake)
add_subdirectory(3rdparty)
add_subdirectory(data)
add_subdirectory(src)
