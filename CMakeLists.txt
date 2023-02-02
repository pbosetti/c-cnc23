cmake_minimum_required(VERSION 3.10)
# NOTE: This project is mostly already configured.
#       Go to the "Targets" section to add new products
#   _____          _ _           _                        
#  |  __ \        | (_)         (_)                       
#  | |__) | __ ___| |_ _ __ ___  _ _ __   __ _ _ __ _   _ 
#  |  ___/ '__/ _ \ | | '_ ` _ \| | '_ \ / _` | '__| | | |
#  | |   | | |  __/ | | | | | | | | | | | (_| | |  | |_| |
#  |_|   |_|  \___|_|_|_| |_| |_|_|_| |_|\__,_|_|   \__, |
#                                                    __/ |
#                                                   |___/ 
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed! Create a build directory and run CMake from there.")
endif()
# This is dynamically updated in defines.h.in:
set(VERSION "1.0")
project(C-CNC VERSION ${VERSION} LANGUAGES C CXX)

#   ____       _   _   _                 
#  / ___|  ___| |_| |_(_)_ __   __ _ ___ 
#  \___ \ / _ \ __| __| | '_ \ / _` / __|
#   ___) |  __/ |_| |_| | | | | (_| \__ \
#  |____/ \___|\__|\__|_|_| |_|\__, |___/
#                              |___/     

#   ____        _ _     _            _   _   _                 
#  | __ ) _   _(_) | __| |  ___  ___| |_| |_(_)_ __   __ _ ___ 
#  |  _ \| | | | | |/ _` | / __|/ _ \ __| __| | '_ \ / _` / __|
#  | |_) | |_| | | | (_| | \__ \  __/ |_| |_| | | | | (_| \__ \
#  |____/ \__,_|_|_|\__,_| |___/\___|\__|\__|_|_| |_|\__, |___/
#                                                    |___/     
# Build type (default to Debug)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type default to Debug" FORCE)
endif(NOT CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  message(STATUS "Debug mode, enabling all warnings")
  add_compile_options(-Wall -Wno-comment)
endif()
# Compiler flags
add_compile_options(-fPIC -D_GNU_SOURCE -Wno-backslash-newline-escape)
# Language Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
# RPATH management
# This embeds the relative path of the dynamic library into the executable
# So that it can find the library across installations
# If this is not given, the executable only searches the library in its same 
# folder or in a system path (/usr/lib, /usr/local/lib)
if (APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else()
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib;/usr/local/lib")
endif()
# FILES
# expect all sources in /src, except mains
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB LIB_SOURCES_CPP "${SOURCE_DIR}/[^_]*.cpp")
file(GLOB LIB_SOURCES "${SOURCE_DIR}/[^_]*.c")
# generate defines.h
configure_file(
  ${SOURCE_DIR}/defines.h.in
  ${SOURCE_DIR}/defines.h
)


#   _____                    _       
#  |_   _|_ _ _ __ __ _  ___| |_ ___ 
#    | |/ _` | '__/ _` |/ _ \ __/ __|
#    | | (_| | | | (_| |  __/ |_\__ \
#    |_|\__,_|_|  \__, |\___|\__|___/
#                 |___/              
# Executables
add_executable(hello ${SOURCE_DIR}/main/hello.c)

# Libraries




#   _____           _        _ _ 
#  |_   _|         | |      | | |
#    | |  _ __  ___| |_ __ _| | |
#    | | | '_ \/ __| __/ _` | | |
#   _| |_| | | \__ \ || (_| | | |
#  |_____|_| |_|___/\__\__,_|_|_|
# List of targets to install
list(APPEND TARGETS_LIST
  hello
)
# Destination directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/products)
install(TARGETS ${TARGETS_LIST}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
)