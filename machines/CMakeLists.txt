cmake_minimum_required(VERSION 2.8)
project(tutorial_asio)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set (CMAKE_CXX_STANDARD 11)
endif ()

set(SOURCE_FILES main.cpp)
add_executable(tutorial_asio ${SOURCE_FILES})
target_link_libraries(tutorial_asio boost_system)
