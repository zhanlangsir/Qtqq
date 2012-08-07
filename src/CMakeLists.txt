set(CORE_HEADERS
	core/groupmsgencoder.h
	core/qqlogincore.h
	core/qqsetting.h
	core/qqmsglistener.h
	core/groupchatlog.h
	core/friendchatlog.h
	core/request.h
	core/msgsender.h
	core/friendmsgencoder.h
	core/types.h
	core/chatmanager.h
	core/sessmsgencoder.h
	core/msgcenter.h
	core/qqavatarrequester.h
	core/parsethread.h
	core/groupimgloader.h
	core/qqmsg.h
	core/nameconvertor.h
	core/groupimgsender.h
	core/qqskinable.h
	core/msgencoder.h
	core/qqskinengine.h
	core/soundplayer.h
	core/qqchatlog.h
	core/imgsender.h
	core/accountmanager.h
	core/pollthread.h
	core/qqitem.h
	core/friendimgsender.h
	core/sockethelper.h
	core/qqchatitem.h
	core/imgloader.h
	core/captchainfo.h
	core/qqutility.h
    core/singleton.h
)

set(UI_HEADERS
	chatlogwin.h
	frienditem.h
	systemtray.h
	qtqq.h
	frienditemmodel.h
	msgtip.h
	grouprequestdlg.h
	qqtextedit.h
	moveablehandler.h
	friendchatdlg.h
	qqtitlebar.h
	recentlistitemmodel.h
	groupitemmodel.h
	sesschatdlg.h
	qqchatdlg.h
	msgbrowse.h
	mainwindow.h
	loginwin.h
	groupchatdlg.h
	qqfacepanel.h
	qqiteminfohelper.h
	qqwidget.h
	traymenuitem.h
	friendrequestdlg.h
	qqitemmodel.h
	traymenu.h
	friendinfodlg.h
    qqglobal.h
    friendsearcher.h
    friendsearchlineedit.h
)

QT4_WRAP_CPP(UI_HEADERS_MOC ${UI_HEADERS})
QT4_WRAP_CPP(CORE_HEADERS_MOC ${CORE_HEADERS})

set(CORE_SOURCES
	core/sessmsgencoder.cpp
	core/qqskinengine.cpp
	core/qqlogincore.cpp
	core/friendimgsender.cpp
	core/qqchatitem.cpp
	core/groupchatlog.cpp
	core/groupmsgencoder.cpp
	core/qqavatarrequester.cpp
	core/imgloader.cpp
	core/types.cpp
	core/groupimgsender.cpp
	core/msgencoder.cpp
	core/friendmsgencoder.cpp
	core/parsethread.cpp
	core/sockethelper.cpp
	core/captchainfo.cpp
	core/qqsetting.cpp
	core/groupimgloader.cpp
	core/pollthread.cpp
	core/msgsender.cpp
	core/soundplayer.cpp
	core/qqitem.cpp
	core/friendchatlog.cpp
	core/qqchatlog.cpp
	core/request.cpp
	core/chatmanager.cpp
	core/imgsender.cpp
	core/qqutility.cpp
	core/accountmanager.cpp
	core/qqmsg.cpp
	core/msgcenter.cpp
)

set(UI_SOURCES
	main.cpp
	qqchatdlg.cpp
	qqtitlebar.cpp
	friendrequestdlg.cpp
	groupchatdlg.cpp
	qtqq.cpp
	chatlogwin.cpp
	systemtray.cpp
	moveablehandler.cpp
	traymenu.cpp
	friendinfodlg.cpp
	traymenuitem.cpp
	qqitemmodel.cpp
	msgbrowse.cpp
	qqwidget.cpp
	mainwindow.cpp
	qqtextedit.cpp
	qqfacepanel.cpp
	msgtip.cpp
	recentlistitemmodel.cpp
	sesschatdlg.cpp
	frienditemmodel.cpp
	grouprequestdlg.cpp
	friendchatdlg.cpp
	loginwin.cpp
	groupitemmodel.cpp
	qqiteminfohelper.cpp
    qqglobal.cpp
    friendsearcher.cpp
    friendsearchlineedit.cpp
)

set(UI_FILES
	ui/captcha.ui
	ui/friendinfodlg.ui
	ui/loginwin.ui
	ui/qqloginwin.ui
	ui/chatlogwin.ui
	ui/friendrequestdlg.ui
	ui/mainwindow.ui
	ui/qqmainwindow.ui
	ui/groupchatdlg.ui
	ui/msgtip.ui
	ui/qqmsgtip.ui
	ui/friendchatdlg.ui
	ui/grouprequestdlg.ui
	ui/qqfacepanel.ui
	ui/sesschatdlg.ui
)

QT4_WRAP_UI(UI_FILES_HEADERS ${UI_FILES})

PKG_CHECK_MODULES(JSONCPP REQUIRED "jsoncpp")
PKG_CHECK_MODULES(ZLIB REQUIRED "zlib")
PKG_CHECK_MODULES(GTK2 REQUIRED "gtk+-2.0")

set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTWEBKIT TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_USE_QTXML TRUE)

include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${LOG4QT_INCLUDE_DIR})
include_directories(${QXGS_INCLUDE_DIR})
include_directories(${GTK2_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(${QT_DEFINITIONS})

set(lang_SOURCES
	${CORE_SOURCES}
	${UI_SOURCES}
	${UI_FILES_HEADERS}
)

set(TRANSLATIONS_FILES ${TRANSLATIONS_DIR}/zh_CN.ts)

# TURN ON UPDATE_TRANSLATIONS will delete all the *.ts files
# BE CAREFUL!
option(UPDATE_TRANSLATIONS "Update source translation files" OFF)

if (UPDATE_TRANSLATIONS)
	QT4_CREATE_TRANSLATION(QM_FILES ${lang_SOURCES} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
	QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS)

add_executable(qtqq
	${CORE_SOURCES} ${CORE_HEADERS_MOC}
	${UI_SOURCES} ${UI_HEADERS_MOC}
	${UI_FILES_HEADERS}
	${QM_FILES}
)

target_link_libraries(qtqq
	log4qt
	qxtglobalshortcut
	${QT_LIBRARIES}
	${JSONCPP_LIBRARIES}
	${GTK2_LIBRARIES}
	${ZLIB_LIBRARIES}
)

install(TARGETS qtqq DESTINATION ${prefix}/bin)
install(FILES ${QM_FILES} DESTINATION ${pkgdatadir}/translations)
