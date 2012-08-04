set(res_DIRS
	avatar
	client_type
	loading
	login_panel
	qqface
	skins
	sound
	status
	ui
)

install(DIRECTORY ${res_DIRS} DESTINATION ${pkgdatadir}/resources)
install(FILES WebQQ.ico DESTINATION ${pkgdatadir}/resources/)
