cmake_minimum_required(VERSION 3.5)

project(Asinc_Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(shared Server.hpp Thread.cpp ThreadPool.cpp Ftp_Server_CMD.cpp Ftp_Task.cpp
    Ftp_Factory.cpp Ftp_List.cpp Ftp_Port.cpp Ftp_Retr.cpp Ftp_Stor.cpp)

add_executable(server main.cpp)

target_link_libraries(server shared event pthread)
