add_subdirectory(icons)

install(FILES qtqq.desktop DESTINATION ${prefix}/share/applications)

set(data_folds 
    misc
    res
    skins
	messagestyle
    )

install(DIRECTORY ${data_folds} DESTINATION ${pkgdatadir})
