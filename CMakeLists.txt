cmake_minimum_required(VERSION 3.20)

project(ChatApp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "adding project include dirs...")
include_directories(
  inc
)

message(STATUS "adding project executable ChatserverMain...")
add_executable(ChatserverMain
  src/shared_state.cpp
  src/listener.cpp
  src/chatserver.cpp
)

message(STATUS "adding project executable ChatclientMain...")
add_executable(ChatclientMain
  src/chatsession.cpp
  src/chatclient.cpp
  src/chatclient_main.cpp
)

# boost
set(BOOST_ROOT "" CACHE PATH ${CMAKE_SOURCE_DIR} / lib)
set(Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost")
find_package(Boost 1.82 REQUIRED COMPONENTS filesystem system thread)
if (Boost_FOUND)
  message(STATUS "including Boost headers...")
  include_directories(
    ${Boost_INCLUDE_DIRS}
  )
  message(STATUS "linking Boost libraries to target: ChatserverMain...")
  target_link_libraries(ChatserverMain
    ${Boost_LIBRARIES}
  )
  message(STATUS "linking Boost libraries to target: ChatclientMain...")
  target_link_libraries(ChatclientMain
    ${Boost_LIBRARIES}
  )
else()
  message(ERROR "Boost package not found...")
endif()


