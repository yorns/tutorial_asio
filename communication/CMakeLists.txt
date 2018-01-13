cmake_minimum_required(VERSION 2.8)
project(wa_clone)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0") # -DWITH_EXEC

set(SOURCE_FILES main.cpp)

add_executable(wa_clone ${SOURCE_FILES})
target_link_libraries(wa_clone boost_system)

add_executable(wa_client main_client.cpp CommandLine.cpp KeyHit.cpp)
target_link_libraries(wa_client boost_system pthread)

add_executable(mthread main_mthread.cpp)
target_link_libraries(mthread boost_system pthread)

add_executable(tcpServer tcpServer.cpp)
target_link_libraries(tcpServer boost_system)

add_executable(allServer all_main.cpp)
target_link_libraries(allServer boost_system)

#add_executable(cmdLine CommandLine.cpp KeyHit.cpp KeyHit.h)
#target_link_libraries(cmdLine boost_system pthread)
