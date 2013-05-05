function(_FIND_DLL_FOR _lib _dll)
	get_filename_component(UTF_BASE_NAME ${_lib} NAME_WE)
	get_filename_component(UTF_PATH ${_lib} PATH)
	find_path(${_lib}_dll_path NAMES ${UTF_BASE_NAME}.dll
		HINTS
			${UTF_PATH}
			${UTF_PATH}/../bin
	)
	if(${_lib}_dll_path)
		set(${_dll} "${${_lib}_dll_path}/${UTF_BASE_NAME}.dll" CACHE INTERNAL "${_dll} Path")
	endif()
	mark_as_advanced(${_lib}_dll_path)
endfunction()

function(_COPY_DLL _target _library _config)
	_FIND_DLL_FOR(${_library} ${_library}_DLL)
	if(${_library}_DLL)
		get_filename_component(UTF_BASE_NAME ${${_library}_DLL} NAME_WE)
        add_custom_command(TARGET ${_target}
			POST_BUILD
            COMMAND if $<CONFIG:${_config}> neq 0 ${CMAKE_COMMAND} -E copy_if_different ${${_library}_DLL} $(OutDir)${UTF_BASE_NAME}.dll
		)
	endif()
endfunction()

macro(_COPY_DLLS _target)
	set(lib_debug ${ARGN})
	set(lib_optimized ${ARGN})
	string(REGEX REPLACE "optimized;[^;]+;?" "" lib_debug "${lib_debug}")
	string(REGEX REPLACE "debug;" ";" lib_debug "${lib_debug}")
	string(REGEX REPLACE "debug;[^;]+;?" "" lib_optimized "${lib_optimized}")
	string(REGEX REPLACE "optimized;" ";" lib_optimized "${lib_optimized}")
	foreach(build_config ${CMAKE_CONFIGURATION_TYPES})
		set(libsToInstall ${lib_optimized})
		if(build_config STREQUAL "Debug")
			set(libsToInstall ${lib_debug})
		endif()
		foreach(lib ${libsToInstall})
			_COPY_DLL(${_target} ${lib} ${build_config})
		endforeach()
	endforeach()
endmacro() 
