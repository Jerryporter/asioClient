cmake_minimum_required(VERSION 3.17)
project(asio)

set(CMAKE_CXX_STANDARD 11)
add_executable(asio main.cpp)

set(BOOST_ROOT C:/Users/chy/Desktop/boost_1_72_0)
set(Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost" )
set(Json_INCLUDE_DIRS C:/Users/chy/Desktop/Lot_c)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    set(Json_INCLUDE_DIRS C:/Users/chy/Desktop/Lot_c)
    include_directories(${Json_INCLUDE_DIRS})
endif()

if(MINGW)
    target_link_libraries(asio ws2_32)
endif()