#------------------------------------------------------------------------------
# General Settings
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)
project(cloak)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_compile_options(-fconcepts-ts)


#------------------------------------------------------------------------------
# Check Dependencies
#------------------------------------------------------------------------------
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)


#------------------------------------------------------------------------------
# Includes
#------------------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE INCLUDE_SRC_COMMON relative ${CMAKE_CURRENT_SOURCE_DIR} "common/*.cpp")

#------------------------------------------------------------------------------
# Executables and Libraries
#------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} cloak.cpp)

add_library(cloak-lib           common/Common.hpp
                                common/Common.cpp
                                common/FdPair.hpp
                                modes/socks5/SocksProxyClient.hpp
                                modes/socks5/SocksProxyClient.cpp
                                modes/socks5/SocksProxyServer.hpp
                                modes/socks5/SocksProxyServer.cpp
                                torptspec/TorPT.hpp
                                torptspec/TorPT.cpp
                                torptspec/TorPTClient.hpp
                                torptspec/TorPTClient.cpp
                                torptspec/TorPTServer.hpp
                                torptspec/TorPTServer.cpp
                                ${INCLUDE_SRC_COMMON}
                                transport/Transport.h
                                transport/Transport.cpp
                                transport/DecodingThread.cpp
                                transport/FragmentManager.cpp
                                transport/EncodingThread.h
                                transport/EncodingThread.cpp
                                controller/ClientController.hpp
                                controller/ClientController.cpp
                                controller/ServerController.hpp
                                controller/ServerController.cpp)




#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

target_link_libraries(${PROJECT_NAME} Boost::system Boost::filesystem cloak-lib pthread)







