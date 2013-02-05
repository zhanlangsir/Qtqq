set(HEADERS
    notifier_plugin.h
    notifytextbrowser.h
    notifywidget.h
    animatedtextbrowser.h
    )

set(SOURCES
    notifier_plugin.cpp
    notifytextbrowser.cpp
    notifywidget.cpp
    animatedtextbrowser.cpp
    )

set(UIS
    notifywidget.ui
    )

include_directories(".")
QT4_WRAP_CPP(MOCA_SOURCES ${HEADERS})
qt4_wrap_ui(NOTIFIER_UI_HEADERS ${UIS})

add_library(notifierplugin SHARED
    ${SOURCES}
    ${MOCA_SOURCES}
    ${NOTIFIER_UI_HEADERS}
    )

set_target_properties(notifierplugin
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_LIBRARY_OUTPUT_PATH}
    )

install(TARGETS notifierplugin LIBRARY DESTINATION ${PLUGIN_DIR} )
