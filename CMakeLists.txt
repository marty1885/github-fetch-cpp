cmake_minimum_required(VERSION 3.5)
project(github-report)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)



find_package(Drogon REQUIRED)
include(DrogonUtilities)

add_executable(${PROJECT_NAME} github-fetch/main.cpp)
target_link_libraries(${PROJECT_NAME} ${DROGON_LIBRARIES} fmt)
if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
	#	target_link_libraries(${PROJECT_NAME} -static)
endif()
