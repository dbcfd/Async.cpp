# - Try to find Gtest, either as library or src. If found, add tests
#
#  This module defines the following variables
#
#  gtest_FOUND - Was library found
#  gtest_INCLUDE_DIRS - the include directories
#
#  This module accepts the following variables
#
#  gtest_DIR - Set to location of DCL if not in PATH or current directory ThirdParty
#  gtest_VERSION - Version of DCL to look for
#  THIRDPARTY_DIR - Location of third party directory to perform checkouts int
#

function(GTEST_ADD_TESTS executable extra_args)
    if(NOT ARGN)
        message(FATAL_ERROR "Missing ARGN: Read the documentation for GTEST_ADD_TESTS")
    endif()
    foreach(source ${ARGN})
        file(READ "${source}" contents)
        string(REGEX MATCHALL "TEST_?F?\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
        foreach(hit ${found_tests})
            string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+), *([A-Za-z_0-9]+) *\\).*" "\\1.\\2" test_name ${hit})
            add_test(${test_name} ${executable} --gtest_filter=${test_name} ${extra_args})
        endforeach()
    endforeach()
endfunction()

macro(UseGtest _tgt _deps)
	if(BUILD_SHARED_LIBS)
		add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY)
	endif()
    if(NOT gtest_FOUND)
        find_package(gtest QUIET)
        if(NOT gtest_FOUND)
            message("-- Searching for gtest as source")
            find_package(GTestSrc REQUIRED)
        endif()
        set(gtest_FOUND ${gtest_FOUND} CACHE INTERNAL "Gtest found")
    endif()
	if(gtest_FOUND)
        include_directories(${gtest_INCLUDE_DIRS})
		GTEST_ADD_TESTS(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_tgt} "" ${ARGN})
		SET(${_deps} ${${_deps}} gtest)
	endif()
endmacro()