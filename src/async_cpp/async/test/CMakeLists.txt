set (TARGET TestAsync)

set(SOURCES
    TestFilter.cpp
    TestMap.cpp
    TestOverload.cpp
    TestParallel.cpp
    TestParallelFor.cpp
    TestParallelForEach.cpp
    TestRunner.cpp
    TestSeries.cpp
    TestUnique.cpp
)

if(Boost_FOUND)
	add_definitions("-DHAS_BOOST")
endif()

include(UseGtest)
UseGtest(${TARGET} DEPENDENCIES ${SOURCES})

SET (DEPENDENCIES ${DEPENDENCIES} Async)

add_executable (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

if(NOT MSVC)
	install (TARGETS ${TARGET} 
		RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
		LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
		ARCHIVE DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
else()
	foreach(config ${CMAKE_CONFIGURATION_TYPES})
		install (TARGETS ${TARGET} 
			RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${config} 
			LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${config} 
			ARCHIVE DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${config})
	endforeach()
endif()

SetVSTargetProperties(${TARGET})