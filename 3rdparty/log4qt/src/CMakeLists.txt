set(log4qt_HEADERS
	log4qt/consoleappender.h
	log4qt/ttcclayout.h
	log4qt/patternlayout.h
	log4qt/simplelayout.h
	log4qt/fileappender.h
	log4qt/dailyrollingfileappender.h
	log4qt/logmanager.h
	log4qt/log4qt.h
	log4qt/propertyconfigurator.h
	log4qt/varia/stringmatchfilter.h
	log4qt/varia/listappender.h
	log4qt/varia/debugappender.h
	log4qt/varia/levelmatchfilter.h
	log4qt/varia/levelrangefilter.h
	log4qt/varia/denyallfilter.h
	log4qt/varia/nullappender.h
	log4qt/level.h
	log4qt/basicconfigurator.h
	log4qt/mdc.h
	log4qt/rollingfileappender.h
	log4qt/spi/filter.h
	log4qt/loggingevent.h
	log4qt/ndc.h
	log4qt/layout.h
	log4qt/loggerrepository.h
	log4qt/appender.h
	log4qt/appenderskeleton.h
	log4qt/writerappender.h
	log4qt/hierarchy.h
	log4qt/logger.h
	log4qt/helpers/classlogger.h
	log4qt/helpers/configuratorhelper.h
	log4qt/helpers/datetime.h
	log4qt/helpers/factory.h
	log4qt/helpers/initialisationhelper.h
	log4qt/helpers/logobject.h
	log4qt/helpers/logobjectptr.h
	log4qt/helpers/optionconverter.h
	log4qt/helpers/patternformatter.h
)	

QT_WRAP_CPP(log4qt_HEADERS_MOC ${log4qt_HEADERS})

set(helpers_SOURCES
	log4qt/helpers/logerror.cpp
	log4qt/helpers/initialisationhelper.cpp
	log4qt/helpers/optionconverter.cpp
	log4qt/helpers/factory.cpp
	log4qt/helpers/logobjectptr.cpp
	log4qt/helpers/configuratorhelper.cpp
	log4qt/helpers/classlogger.cpp
	log4qt/helpers/properties.cpp
	log4qt/helpers/datetime.cpp
	log4qt/helpers/patternformatter.cpp
	log4qt/helpers/logobject.cpp
)

set(spi_SOURCES
	log4qt/spi/filter.cpp
)

set(varia_SOURCES
	log4qt/varia/denyallfilter.cpp
	log4qt/varia/nullappender.cpp
	log4qt/varia/levelrangefilter.cpp
	log4qt/varia/stringmatchfilter.cpp
	log4qt/varia/levelmatchfilter.cpp
	log4qt/varia/listappender.cpp
	log4qt/varia/debugappender.cpp
)

set(log4qt_SOURCES
	log4qt/fileappender.cpp
	log4qt/hierarchy.cpp
	log4qt/appenderskeleton.cpp
	log4qt/rollingfileappender.cpp
	log4qt/basicconfigurator.cpp
	log4qt/consoleappender.cpp
	log4qt/writerappender.cpp
	log4qt/layout.cpp
	log4qt/propertyconfigurator.cpp
	log4qt/loggerrepository.cpp
	log4qt/mdc.cpp
	log4qt/log4qt.cpp
	log4qt/ttcclayout.cpp
	log4qt/loggingevent.cpp
	log4qt/simplelayout.cpp
	log4qt/dailyrollingfileappender.cpp
	log4qt/ndc.cpp
	log4qt/logmanager.cpp
	log4qt/level.cpp
	log4qt/logger.cpp
	log4qt/patternlayout.cpp
)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(log4qt SHARED
	${helpers_SOURCES}
	${spi_SOURCES}
	${varia_SOURCES}
	${log4qt_SOURCES}
    ${log4qt_HEADERS}
	${log4qt_HEADERS_MOC}
)

install(TARGETS log4qt LIBRARY DESTINATION ${libdir})
