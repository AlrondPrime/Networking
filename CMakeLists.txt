include(Properties.cmake)


cmake_minimum_required(VERSION 3.25)
set(CMAKE_C_COMPILER    "C:/Program Files/LLVM/bin/clang.exe")
set(CMAKE_CXX_COMPILER  "C:/Program Files/LLVM/bin/clang++.exe")
set(CMAKE_RC_COMPILER  "llvm-rc")


project(Networking)
set(CMAKE_CXX_STANDARD 17)

set(Boost_NO_WARN_NEW_VERSIONS 1)
#set(Boost_DEBUG ON)
#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.81.0 COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(PCH src/pch.h)
set(NETWORKING_COMMON src/net/Message.hpp src/net/ts_deque.hpp)
set(NETWORKING_CLIENT src/net/Client.hpp src/net/Connection.hpp)
set(NETWORKING_SERVER src/net/Server.hpp)

add_executable(Client src/Client.cpp ${NETWORKING_CLIENT} ${NETWORKING_COMMON})
target_link_libraries(Client ${Boost_LIBRARIES})
target_precompile_headers(Client
        PRIVATE ${PCH})

add_executable(Server src/Server.cpp ${NETWORKING_SERVER} ${NETWORKING_COMMON})
target_link_libraries(Server ${Boost_LIBRARIES})
target_precompile_headers(Server
        PRIVATE ${PCH})
