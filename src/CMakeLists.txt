set(CORE_HEADERS
	core/qqlogincore.h
	core/qqmsglistener.h
	core/groupchatlog.h
	core/friendchatlog.h
	core/request.h
    core/qqavatarrequester.h
	core/qqmsg.h
	core/qqskinable.h
	core/qqchatlog.h
	core/accountmanager.h
	core/qqitem.h
	core/sockethelper.h
	core/qqchatitem.h
	core/captchainfo.h
	core/qqutility.h
    core/singleton.h
    core/tasks.h
	core/talkable.h
	core/curr_login_account.h

	#chat widget
	chatwidget/tabwindow.h
	chatwidget/qqchatdlg.h
	chatwidget/friendchatdlg.h
	chatwidget/groupchatdlg.h
	chatwidget/sesschatdlg.h
	chatwidget/chatmsg_processor.h
	chatwidget/chatdlg_manager.h

    #event handle
    event_handle/event_handle.h

    #file transfer
    file_transfer/file_transfer_manager.h
    file_transfer/file_transfer_dlg.h
    file_transfer/sendfile_widget.h
    file_transfer/recvfile_widget.h

    #hotkey manager
    hotkeymanager/hotkey_manager.h
    hotkeymanager/hotkeys.h

    #interfaces/
    interfaces/iobserver.h
    interfaces/isubject.h
    
	#request widget
	requestwidget/grouprequestdlg.h
	requestwidget/friendrequestdlg.h
	requestwidget/requestmsg_processor.h

	#msgprocessor
	msgprocessor/msg_processor.h

    #notification manager
    notification_manager/notification_manager.h

    #plugin manager
    pluginmanager/plugin_manager.h
    pluginmanager/plugin_managedlg.h

	#qq_protocol
    protocol/event.h
	protocol/qq_protocol.h
	protocol/pollthread.h
    protocol/event_center.h
	protocol/imgsender.h
	protocol/filesender.h
    protocol/msgsender.h

	#request_jobs
	protocol/request_jobs/friendinfo2_job.h
	protocol/request_jobs/group_memberlist_job.h
	protocol/request_jobs/icon_job.h
	protocol/request_jobs/job_base.h
	protocol/request_jobs/sendmsg_job.h
    protocol/request_jobs/sendimg_job.h
    protocol/request_jobs/sendfile_job.h
    protocol/request_jobs/filerecive_job.h
	protocol/request_jobs/strangerinfo2_job.h
	protocol/request_jobs/loadimg_job.h
	protocol/request_jobs/refusefile_job.h

	#roster
	roster/roster.h
    roster/group_presister.h

	#roster model
	rostermodel/roster_model_base.h
	rostermodel/roster_index.h
	rostermodel/roster_model.h
	rostermodel/recent_model.h
	rostermodel/contact_proxy_model.h
    rostermodel/contact_searcher.h

    #setting
    setting/setting.h

	#skinengine
	skinengine/qqskinengine.h
	skinengine/msgstyle_manager.h

    #snapshot
    snapshot/freeregiongrabber.h
    snapshot/freeregiongrabber.h
    snapshot/ksnapshot.h
    snapshot/ksnapshotobject.h
    snapshot/ksnapshotpreview.h
    snapshot/regiongrabber.h
    snapshot/snapshottimer.h
    snapshot/windowgrabber.h


	#sound player
	soundplayer/soundplayer.h

	#storage
	storage/storage.h

	#strangermanager
	strangermanager/stranger_manager.h

	#utils
	utils/menu.h
	utils/contact_status.h
	utils/icon_decorator.h
    utils/htmltomsgparser.h
    utils/file_ask_dlg.h

	#trayicon
	trayicon/systemtray.h
)

set(UI_HEADERS
	chatlogwin.h
	qtqq.h
	qqtextedit.h
	moveablehandler.h
	qqtitlebar.h
	msgbrowse.h
	mainwindow.h
	loginwin.h
	qqfacepanel.h
	qqiteminfohelper.h
	qqwidget.h
	friendinfodlg.h
    qqglobal.h
)

QT4_WRAP_CPP(UI_HEADERS_MOC ${UI_HEADERS})
QT4_WRAP_CPP(CORE_HEADERS_MOC ${CORE_HEADERS})

set(CORE_SOURCES
	core/qqlogincore.cpp
	core/qqchatitem.cpp
    core/qqavatarrequester.cpp
	core/groupchatlog.cpp
	core/sockethelper.cpp
	core/captchainfo.cpp
	core/qqitem.cpp
	core/friendchatlog.cpp
	core/qqchatlog.cpp
	core/request.cpp
	core/qqutility.cpp
	core/accountmanager.cpp
	core/qqmsg.cpp
	core/curr_login_account.cpp
    core/talkable.cpp

    #event handle
    event_handle/event_handle.cpp

    #file transfer
    file_transfer/file_transfer_manager.cpp
    file_transfer/file_transfer_dlg.cpp
    file_transfer/sendfile_widget.cpp
    file_transfer/recvfile_widget.cpp

    #hotkey manager
    hotkeymanager/hotkey_manager.cpp

	#msgprocessor
	msgprocessor/msg_processor.cpp

    #notification manager
    notification_manager/notification_manager.cpp

    #plugin manager
    pluginmanager/plugin_manager.cpp
    pluginmanager/plugin_managedlg.cpp

	#qq_protocol
	protocol/qq_protocol.cpp
	protocol/pollthread.cpp
    protocol/event_center.cpp
	protocol/imgsender.cpp
	protocol/filesender.cpp
    protocol/msgsender.cpp

    #request_jobs
	protocol/request_jobs/friendinfo2_job.cpp
	protocol/request_jobs/group_memberlist_job.cpp
	protocol/request_jobs/icon_job.cpp
	protocol/request_jobs/job_base.cpp
	protocol/request_jobs/sendmsg_job.cpp
    protocol/request_jobs/sendimg_job.cpp
    protocol/request_jobs/sendfile_job.cpp
    protocol/request_jobs/filerecive_job.cpp
	protocol/request_jobs/strangerinfo2_job.cpp
	protocol/request_jobs/loadimg_job.cpp
	protocol/request_jobs/refusefile_job.cpp

	#roster
	roster/roster.cpp
    roster/group_presister.cpp

	#roster model
	rostermodel/roster_model_base.cpp
	rostermodel/recent_model.cpp
	rostermodel/roster_index.cpp
	rostermodel/roster_model.cpp
	rostermodel/contact_proxy_model.cpp
    rostermodel/contact_searcher.cpp

    #setting
    setting/setting.cpp

	#skinengine
	skinengine/qqskinengine.cpp
	skinengine/msgstyle_manager.cpp

    #snapshot
    snapshot/expblur.cpp
    snapshot/freeregiongrabber.cpp
    snapshot/freeregiongrabber.cpp
    snapshot/ksnapshot.cpp
    snapshot/ksnapshotobject.cpp
    snapshot/ksnapshotpreview.cpp
    snapshot/regiongrabber.cpp
    snapshot/snapshottimer.cpp
    snapshot/windowgrabber.cpp

	#sound player
	soundplayer/soundplayer.cpp

	#storage
	storage/storage.cpp

	#strangermanager
	strangermanager/stranger_manager.cpp

	#utils
	utils/icon_decorator.cpp
	utils/menu.cpp
    utils/file_ask_dlg.cpp

	#chat widget
	chatwidget/tabwindow.cpp
	chatwidget/friendchatdlg.cpp
	chatwidget/groupchatdlg.cpp
	chatwidget/qqchatdlg.cpp
	chatwidget/sesschatdlg.cpp
	chatwidget/chatmsg_processor.cpp
	chatwidget/chatdlg_manager.cpp

	#request widget
	requestwidget/grouprequestdlg.cpp
	requestwidget/friendrequestdlg.cpp
	requestwidget/requestmsg_processor.cpp

	#trayicon
	trayicon/systemtray.cpp
)

set(UI_SOURCES
	main.cpp
	qqtitlebar.cpp
	qtqq.cpp
	chatlogwin.cpp
	moveablehandler.cpp
	friendinfodlg.cpp
	msgbrowse.cpp
	qqwidget.cpp
	mainwindow.cpp
	qqtextedit.cpp
	qqfacepanel.cpp
	loginwin.cpp
	qqiteminfohelper.cpp
    qqglobal.cpp
)

set(UI_FILES
	ui/captcha.ui
	ui/friendinfodlg.ui
	ui/loginwin.ui
	ui/chatlogwin.ui
	ui/friendrequestdlg.ui
	ui/mainwindow.ui
	ui/groupchatdlg.ui
	ui/friendchatdlg.ui
	ui/grouprequestdlg.ui
	ui/qqfacepanel.ui
	ui/sesschatdlg.ui
	ui/tabwindow.ui
    ui/pluginmanagedlg.ui
    ui/fileaskdlg.ui
    ui/filetransferdlg.ui
    ui/ksnapshotwidget.ui
    ui/hotkeydlg.ui
    ui/sendfilewidget.ui
    ui/recvfilewidget.ui
)

QT4_WRAP_UI(UI_FILES_HEADERS ${UI_FILES})

PKG_CHECK_MODULES(ZLIB REQUIRED "zlib")

set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTWEBKIT TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_USE_QTXML TRUE)

include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${JSONCPP_INCLUDE_DIR})
include_directories(${LOG4QT_INCLUDE_DIR})
include_directories(${QXGS_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(${QT_DEFINITIONS})

#set(lang_SOURCES
#	${CORE_SOURCES}
#	${UI_SOURCES}
#	${UI_FILES_HEADERS}
#)

#set(TRANSLATIONS_FILES ${TRANSLATIONS_DIR}/qtqq_zh.ts)
set(TRANSLATIONS_FILE ${TRANSLATIONS_DIR}/qtqq_zh.qm)

# TURN ON UPDATE_TRANSLATIONS will delete all the *.ts files
# BE CAREFUL!
option(UPDATE_TRANSLATIONS "Update source translation files" OFF)

#if (UPDATE_TRANSLATIONS)
#	QT4_CREATE_TRANSLATION(QM_FILES ${lang_SOURCES} ${TRANSLATIONS_FILES})
#else (UPDATE_TRANSLATIONS)
#	QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})
#endif (UPDATE_TRANSLATIONS)

add_subdirectory(plugins)

add_executable(qtqq
	${CORE_SOURCES} ${CORE_HEADERS_MOC}
	${UI_SOURCES} ${UI_HEADERS_MOC}
	${UI_FILES_HEADERS}
	${QM_FILES}
)

if (X11_Xfixes_FOUND)
    target_link_libraries(qtqq ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

target_link_libraries(qtqq
	log4qt
	qxtglobalshortcut
	jsoncxx
	${QT_LIBRARIES}
	${ZLIB_LIBRARIES}
)

install(TARGETS qtqq DESTINATION ${prefix}/bin)
#install(FILES ${QM_FILES} DESTINATION ${pkgdatadir}/translations)
install(FILES ${TRANSLATIONS_FILE} DESTINATION ${pkgdatadir}/translations)
