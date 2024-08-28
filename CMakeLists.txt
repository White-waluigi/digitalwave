cmake_minimum_required(VERSION 3.15)

project(DWExcercise)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
# Find and link libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

target_compile_options(main PRIVATE -Wall -Wextra -pedantic -g)
# list of source files
set(SOURCE_FILES
	main.cpp
	MurmurHash3.cpp
)

# Add murmur hash cpp
add_executable(main ${SOURCE_FILES})

target_link_libraries(main ${CURL_LIBRARIES})
