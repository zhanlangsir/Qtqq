foreach(size 16 24 32 48 64 128 256)
	install(DIRECTORY ${size}x${size} DESTINATION ${prefix}/share/icons/hicolor
		PATTERN .* EXCLUDE
		PATTERN *~ EXCLUDE)
endforeach(size 16 24 32 48 64 128 256)
