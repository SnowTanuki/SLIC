cmake_minimum_required(VERSION 3.1)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-O3 -Wall")
endif()

# OpenMP
find_package(OpenMP REQUIRED)

# OpenCV
find_package(OpenCV REQUIRED)
if (OpenCV_VERSION VERSION_LESS 3.0)
	message(FATAL_ERROR "Error: OpenCV version requires at least 3.0")
endif()

include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB SRCS ./*.cpp ./*.hpp)

add_executable(slic_test ${COMMON_SRCS} ${SRCS})
target_link_libraries(slic_test ${OpenCV_LIBS})