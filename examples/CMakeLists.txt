#   _____                           _           
#  | ____|_  ____ _ _ __ ___  _ __ | | ___  ___ 
#  |  _| \ \/ / _` | '_ ` _ \| '_ \| |/ _ \/ __|
#  | |___ >  < (_| | | | | | | |_) | |  __/\__ \
#  |_____/_/\_\__,_|_| |_| |_| .__/|_|\___||___/
#                            |_|                
cmake_minimum_required(VERSION 3.10)
project("C-CNC tests" VERSION 1.0 LANGUAGES C)

#   ____       _   _   _                 
#  / ___|  ___| |_| |_(_)_ __   __ _ ___ 
#  \___ \ / _ \ __| __| | '_ \ / _` / __|
#   ___) |  __/ |_| |_| | | | | (_| \__ \
#  |____/ \___|\__|\__|_|_| |_|\__, |___/
#                              |___/     
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

if(APPLE)
  # On Intel Macs, brew uses /usr/local
  if(EXISTS /usr/local)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
  endif()
  # on M1 Macs, brew uses /opt/homebrew:
  if(EXISTS /opt/homebrew)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
  endif()
endif()

#   _____                    _       
#  |_   _|_ _ _ __ __ _  ___| |_ ___ 
#    | |/ _` | '__/ _` |/ _ \ __/ __|
#    | | (_| | | | (_| |  __/ |_\__ \
#    |_|\__,_|_|  \__, |\___|\__|___/
#                 |___/              
add_executable(0_hello ${SOURCES}/0_hello.c)
add_executable(1_types ${SOURCES}/1_types.c)
add_executable(2_basics ${SOURCES}/2_basics.c)
add_executable(3_file ${SOURCES}/3_file.c)
add_executable(4_include_guards ${SOURCES}/4_include_guards.c)
add_executable(5_struct_union ${SOURCES}/5_struct_union.c)
add_executable(6_inout_p ${SOURCES}/6_inout_p.c ${SOURCES}/inout.c)
add_executable(7_linkedlist ${SOURCES}/7_linkedlist.c)
add_executable(8_timed ${SOURCES}/8_timed.c ${SOURCES}/stats.c)
target_link_libraries(8_timed m pthread)
add_executable(9_timed ${SOURCES}/9_timed.c ${SOURCES}/stats.c)
target_link_libraries(9_timed m)
add_executable(10_timed ${SOURCES}/10_timed.c ${SOURCES}/stats.c)
target_link_libraries(10_timed m)
add_executable(12_encoding ${SOURCES}/12_encoding.c)

add_executable(vending ${SOURCES}/fsm.c ${SOURCES}/vending.c)