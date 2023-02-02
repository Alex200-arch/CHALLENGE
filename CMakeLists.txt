cmake_minimum_required(VERSION 3.10)
project(msg-lite CXX)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES src/log.cpp src/message_handler.cpp)

set(COMMON_LIBS
    libspdlog.a)

include_directories(spdlog/include)
include_directories(include)
link_directories(${PROJECT_SOURCE_DIR}/spdlog/build)

add_executable(ms-lite ms-lite.cpp ${COMMON_SOURCES} src/server_application.cpp src/message_server.cpp src/message_server_socket.cpp)
target_link_libraries(ms-lite PRIVATE pthread ${COMMON_LIBS})

add_executable(mc-lite mc-lite.cpp ${COMMON_SOURCES} src/client_application.cpp src/message_client_socket.cpp)
target_link_libraries(mc-lite PRIVATE ${COMMON_LIBS})
