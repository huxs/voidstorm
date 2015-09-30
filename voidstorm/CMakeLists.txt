# CMake settings
cmake_minimum_required(VERSION 2.8.11)

set(PROJECT_NAME voidstorm)
project(${PROJECT_NAME})

file(GLOB_RECURSE HEADERS code/*.h)

if(WIN32)
file(GLOB_RECURSE SOURCES code/windows/*.cpp)
elseif(UNIX)
file(GLOB_RECURSE SOURCES code/linux/*.cpp)
endif()

set(SOURCES ${SOURCES} code/voidstorm.cpp)

set(LIBRARIES ${EXTERNAL_LIBRARIES})

include_directories(code)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
