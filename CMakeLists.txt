cmake_minimum_required(VERSION 3.15)

project(DWExcercise)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
# Find and link libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

# list of source files
set(SOURCE_FILES
	task1.cpp
	MurmurHash3.cpp
)

# Add murmur hash cpp
add_executable(task1 ${SOURCE_FILES})

target_link_libraries(task1 ${CURL_LIBRARIES})
target_compile_options(task1 PRIVATE -Wall -Wextra -pedantic -g)



# list of source files
set(SOURCE_FILES
	task2.cpp
)

# Add murmur hash cpp
add_executable(task2 ${SOURCE_FILES})

target_link_libraries(task2 ${CURL_LIBRARIES})
target_compile_options(task2 PRIVATE -Wall -Wextra -pedantic -g)
