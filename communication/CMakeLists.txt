cmake_minimum_required(VERSION 2.8)
project(asioExamples)

set(BOOSTROOT /opt/boost)

find_package( Threads REQUIRED)
find_package( Boost 1.65.0 REQUIRED COMPONENTS system coroutine  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -O0") # -DWITH_EXEC

set(SOURCE_FILES main.cpp)

add_executable(wa_clone ${SOURCE_FILES})
target_link_libraries(wa_clone ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(wa_clone PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(wa_client main_client.cpp CommandLine.cpp KeyHit.cpp)
target_link_libraries(wa_client ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(wa_client PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(mthread_bad main_mthread.cpp)
target_link_libraries(mthread_bad ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(mthread_bad PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(mthread main_mthread.cpp)
target_link_libraries(mthread ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(mthread PUBLIC ${Boost_INCLUDE_DIRS} )
target_compile_definitions(mthread PUBLIC WITH_STRAND)

add_executable(tcpServer tcpServer.cpp)
target_link_libraries(tcpServer ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(tcpServer PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(allServer all_main.cpp)
target_link_libraries(allServer ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(allServer PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(httpRestClient http_client_sync.cpp)
target_link_libraries(httpRestClient ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(httpRestClient PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(httpRestClient2 http_client_coro2.cpp)
target_link_libraries(httpRestClient2 ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(httpRestClient2 PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(httpRestClientAsync http_client_async.cpp)
target_link_libraries(httpRestClientAsync ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(httpRestClientAsync PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(websocketServer websocket_server_async.cpp)
target_link_libraries(websocketServer ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(websocketServer PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(websocketClient websocket_client_sync.cpp)
target_link_libraries(websocketClient ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(websocketClient PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(xorMe xorMe.cpp)

add_executable(beastHttpServer beast_http_server.cpp)
target_link_libraries(beastHttpServer ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(beastHttpServer PUBLIC ${Boost_INCLUDE_DIRS} )

add_executable(beastHttpServer2 http_server_coro.cpp)
target_link_libraries(beastHttpServer2 ${Boost_LIBRARIES} Threads::Threads)
target_include_directories(beastHttpServer2 PUBLIC ${Boost_INCLUDE_DIRS} )

#add_executable(cmdLine CommandLine.cpp KeyHit.cpp KeyHit.h)
#target_link_libraries(cmdLine boost_system pthread)
