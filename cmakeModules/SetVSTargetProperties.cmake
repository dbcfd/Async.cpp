MACRO(SetVSTargetProperties PROJECT_NAME)
	IF(MSVC)
		set_target_properties( ${PROJECT_NAME} PROPERTIES 
			LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
			LINK_FLAGS "/ignore:4221"
			STATIC_LIBRARY_FLAGS "/ignore:4221"
		)
	ENDIF()
ENDMACRO()