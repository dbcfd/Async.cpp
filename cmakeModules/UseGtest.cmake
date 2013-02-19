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

macro(UseGtest _tgt _deps)
	if(BUILD_SHARED_LIBS)
		add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY)
	endif()
	find_package(gtest QUIET)
	if(NOT gtest_FOUND)
	    message("-- Searching for gtest as source")
		find_package(GTestSrc REQUIRED)
	endif()
	if(gtest_FOUND)
		GTEST_ADD_TESTS(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_tgt} "" ${ARGN})
		SET(${_deps} ${${_deps}} gtest)
	endif()
endmacro()