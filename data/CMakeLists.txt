add_subdirectory(resources)
add_subdirectory(skins)
add_subdirectory(misc)
add_subdirectory(icons)

set(DESKTOP_FILES
	qtqq.desktop
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/qtqq.desktop DESTINATION ${prefix}/share/applications)
