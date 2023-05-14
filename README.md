# Networking
Simple tcp clien-server application written in C++ using Boost.Asio
## Usage
Compile applications using clang++ compiler

Launch server and client applications with command-line arguments like below

```
Server.exe ..\\ServerStorage
Client.exe ..\\ClientStorage
```
## Warning
In CMakeLists.txt file check if `CMAKE_C_COMPILER`, `CMAKE_CXX_COMPILER` and `CMAKE_RC_COMPILER` variables are set correctly

You need to manually create file `Properties.cmake` and set Cmake variables like below

```
set(BOOST_ROOT <path_to_your_Boost_library>)
set(BOOST_LIBRARYDIR <path_to_your_Boost_library>)
```
