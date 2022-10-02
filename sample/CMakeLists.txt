project(slic)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-O3 -Wall")
endif()

# OpenCV
find_package(OpenCV REQUIRED)
if (OpenCV_VERSION VERSION_LESS 3.0)
	message(FATAL_ERROR "Error: OpenCV version requires at least 3.0")
endif()

file(GLOB srcs ${PROJECT_SOURCE_DIR}/../src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../sample/sample.cpp ${srcs})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(${CMAKE_PROJECT_NAME} ${OpenCV_LIBS})
