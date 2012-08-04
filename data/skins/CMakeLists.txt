set(skin_DIRS
	default
	messagestyle
)

install(DIRECTORY ${skin_DIRS}
	DESTINATION ${pkgdatadir}/skins
)
