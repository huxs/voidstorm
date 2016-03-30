if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

# CMake settings
cmake_minimum_required(VERSION 2.8.11)
set(PROJECT_NAME Voidstorm)
project(${PROJECT_NAME})

# Find external libraries and modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/ ${CMAKE_SOURCE_DIR}/CMake/rpavlik)
include(CreateLaunchers)

# Check 64 bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message("Generating x64 project.")					   
  set(HAVE_64_BIT 1)
  add_definitions(-D_HAVE_X64)
else()
  message("Generating x86 project.")					   
  set(HAVE_64_BIT 0)
endif()

# Path to where targets will be built
if(HAVE_64_BIT EQUAL 1)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin64)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib32)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib32)
endif()

# Check platform
if(WIN32)
  add_definitions(-DWINDOWS)
elseif(UNIX AND NOT APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    add_definitions(-DLINUX)
    if(NOT HAVE_64_BIT)
      set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
    endif()
  endif()
endif()

# MSVC
if(MSVC)
  #add_definitions(/fp:fast)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS=1) # Disable warnings for calling unsafe methods in the C++ Standard Library
  add_definitions(-D_SECURE_SCL=0)             # Disable Checked iterators
  add_definitions(-D_HAS_EXCEPTIONS=0)         # Disable exceptions in runtime library
  add_definitions(-D_HAS_ITERATOR_DEBUGGING=0) # Disable itterator debugging in runtime library
  add_definitions(-DWIN32_LEAN_AND_MEAN=1)     # Excludes certain Win32 APIs
  add_definitions(-DUNICODE -D_UNICODE)        # Unicode support instead of ANSI
  add_definitions(/EHa-)                       # Disable exeception handling
  add_definitions(/GR-)                        # Disable RTTI
  add_definitions(/MP)                         # Enable Multicore compiling
  add_definitions(/WX)                         # Treat warnings as errors
  add_definitions(/wd4005)                     # Disable macro redefinition warning
  add_definitions(/wd4297)                     # Disable SDL throw warning

  set(CMAKE_EXE_LINKER_FLAGS "/ignore:4098" )  # Disable the warning for mixing runtime libraries
  set(CMAKE_EXE_LINKER_FLAGS "/subsystem:windows" )  
endif()

# GCC
if(CMAKE_COMPILER_IS_GNUCC)
  # set(CMAKE_VERBOSE_MAKEFILE TRUE)  
  add_definitions(-D__STDC_LIMIT_MACROS=1)
  add_definitions(-D__STDC_CONSTANT_MACROS=1)
  add_definitions(-DGCC)
  add_definitions(-std=c++11)
  add_definitions(-msse2)
  add_definitions(-mmmx)
endif()

add_definitions(-DVOIDSTORM_INTERNAL)
add_definitions(-DVOIDSTORM_DATA_DIRECTORY="${CMAKE_SOURCE_DIR}/data/")
add_definitions(-DVOIDSTORM_SHADER_DIRECTORY="${CMAKE_SOURCE_DIR}/data/shaders/")
add_definitions(-DVOIDSTORM_SCRIPT_DIRECTORY="${CMAKE_SOURCE_DIR}/data/scripts/")
add_definitions(-DVOIDSTORM_TEXTURE_DIRECTORY="${CMAKE_SOURCE_DIR}/data/textures/")
add_definitions(-DVOIDSTORM_FONT_DIRECTORY="${CMAKE_SOURCE_DIR}/data/fonts/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find modules.
find_package(DCFX REQUIRED)
find_package(LUA REQUIRED)
find_package(DLMalloc REQUIRED)

# Set external libs.
if(HAVE_64_BIT)
  set(EXTERNAL_LIBRARIES
    optimized ${DCFX_LIBRARY_x64} debug ${DCFX_LIBRARY_x64}
    optimized ${DCUTIL_LIBRARY_x64} debug ${DCUTIL_LIBRARY_x64}
    optimized ${LUA_LIBRARY_x64} debug ${LUA_LIBRARY_x64}
    optimized ${DLMALLOC_LIBRARY_x64} debug ${DLMALLOC_LIBRARY_x64}
    )
else()
  set(EXTERNAL_LIBRARIES
    optimized ${DCFX_LIBRARY_x86} debug ${DCFX_LIBRARY_x86}
    optimized ${DCUTIL_LIBRARY_x86} debug ${DCUTIL_LIBRARY_x86}
    optimized ${LUA_LIBRARY_x86} debug ${LUA_LIBRARY_x86}
    optimized ${DLMALLOC_LIBRARY_x86} debug ${DLMALLOC_LIBRARY_x86}
    )
endif()

include_directories(include)

add_subdirectory(voidstorm)

# Copy dlls to output directory.
if(HAVE_64_BIT)
  add_custom_target(_DependenciesDEBUG ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dll/x64 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  create_default_target_launcher(_DependenciesDEBUG WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin64)
  add_custom_target(_DependenciesRELEASE ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dll/x64 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  create_default_target_launcher(_DependenciesRELEASE WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin64)
else()
  add_custom_target(_DependenciesDEBUG ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dll/x86 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  create_default_target_launcher(_DependenciesDEBUG WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin32)
  add_custom_target(_DependenciesRELEASE ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dll/x86 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  create_default_target_launcher(_DependenciesRELEASE WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin32)
endif()
