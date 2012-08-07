add_subdirectory(icons)

install(FILES qtqq.desktop DESTINATION ${prefix}/share/applications)

set(data_folds 
    misc
    resources
    skins
    )

install(DIRECTORY ${data_folds} DESTINATION ${pkgdatadir})
