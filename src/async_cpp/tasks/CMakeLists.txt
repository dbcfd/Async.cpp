set (TARGET Tasks)

SET (DEPENDENCIES)

if(Boost_FOUND)
	add_definitions("-DHAS_BOOST")
	include_directories(${Boost_INCLUDE_DIRS})
	set(DEPENDENCIES ${DEPENDENCIES} ${Boost_LIBRARIES})
endif()

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

add_library (${TARGET} ${LIBRARY_BUILD_TYPE} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION}
	DEBUG_OUTPUT_NAME ${TARGET}${DEBUG_LIB_CONVENTION}
	OUTPUT_NAME ${TARGET}${LIB_CONVENTION})

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${BASE_DIRECTORY}/install/${CMAKE_PROJECT_NAME}/${version}/include/async_cpp FILES_MATCHING PATTERN "*.h" )
install (TARGETS ${TARGET} 
	RUNTIME DESTINATION ${BASE_DIRECTORY}/install/${CMAKE_PROJECT_NAME}/${version}/bin
	LIBRARY DESTINATION ${BASE_DIRECTORY}/install/${CMAKE_PROJECT_NAME}/${version}/lib
	ARCHIVE DESTINATION ${BASE_DIRECTORY}/install/${CMAKE_PROJECT_NAME}/${version}/lib)

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()