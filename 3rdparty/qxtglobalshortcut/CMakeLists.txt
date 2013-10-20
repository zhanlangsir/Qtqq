set(qxtglobalshortcut_HEADERS
	qxtglobal.h
	qxtglobalshortcut.h
	qxtglobalshortcut_p.h
)

set(qxtglobalshortcut_SOURCES
	qxtglobalshortcut.cpp
	qxtglobalshortcut_x11.cpp
)

PKG_CHECK_MODULES(X11 REQUIRED "x11")

include_directories(X11_INCLUDE_DIRS)
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5X11Extras_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

QT_WRAP_CPP(qxtglobalshortcut_HEADERS_MOC ${qxtglobalshortcut_HEADERS})

add_library(qxtglobalshortcut
	SHARED
	${qxtglobalshortcut_SOURCES}
	${qxtglobalshortcut_HEADERS_MOC}
)

target_link_libraries(qxtglobalshortcut
	${X11_LIBRARIES}
)

install(TARGETS qxtglobalshortcut LIBRARY DESTINATION ${libdir})
