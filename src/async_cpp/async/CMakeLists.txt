set (TARGET Async)

set(DETAIL_HEADERS
    detail/IParallelTask.h
    detail/ISeriesTask.h
    detail/ParallelCollectTask.h
    detail/ParallelTask.h
    detail/ParallelTerminalTask.h
    detail/SeriesCollectTask.h
    detail/SeriesTask.h
    detail/SeriesTerminalTask.h
)

set(DETAIL_SOURCES
    detail/IParallelTask.cpp
    detail/ISeriesTask.cpp
    detail/ParallelCollectTask.cpp
    detail/ParallelTask.cpp
    detail/ParallelTerminalTask.cpp
    detail/SeriesCollectTask.cpp
    detail/SeriesTask.cpp
    detail/SeriesTerminalTask.cpp
)

source_group("Header Files\\Detail" FILES ${DETAIL_HEADERS})
source_group("Source Files\\Detail" FILES ${DETAIL_SOURCES})

set(HEADERS
    Async.h
    AsyncResult.h
    Filter.h
    Map.h
    Parallel.h
    ParallelFor.h
    ParallelForEach.h
    Series.h
    Unique.h
)

set(SOURCES
    AsyncResult.cpp
    Filter.cpp
    Map.cpp
    Parallel.cpp
    ParallelFor.cpp
    ParallelForEach.cpp
    Series.cpp
    Unique.cpp
)

SET (DEPENDENCIES Tasks)

add_library (${TARGET} ${LIBRARY_BUILD_TYPE} ${HEADERS} ${DETAIL_HEADERS} ${SOURCES} ${DETAIL_SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION}
	DEBUG_OUTPUT_NAME ${TARGET}${DEBUG_LIB_CONVENTION}
	OUTPUT_NAME ${TARGET}${LIB_CONVENTION})

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/include/async_cpp FILES_MATCHING PATTERN "*.h" )
install (TARGETS ${TARGET} 
	RUNTIME DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/bin
	LIBRARY DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib
	ARCHIVE DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib)


SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()