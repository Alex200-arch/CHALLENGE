cmake_minimum_required(VERSION 3.10)
project(msg-lite CXX)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES src/log.cpp src/message_handler.cpp)

set(COMMON_LIBS
    libspdlog.a)

include_directories(include spdlog/include)
link_directories(${PROJECT_SOURCE_DIR}/spdlog/build)

add_executable(ms-lite ms-lite.cpp ${COMMON_SOURCES} src/server_application.cpp src/message_server.cpp src/message_server_socket.cpp)
target_link_libraries(ms-lite PRIVATE pthread ${COMMON_LIBS})

add_executable(mc-lite mc-lite.cpp ${COMMON_SOURCES} src/client_application.cpp src/message_client_socket.cpp)
target_link_libraries(mc-lite PRIVATE ${COMMON_LIBS})

# unit test
set(GTEST_LIBS
    libgtest.a
    libgtest_main.a)
add_executable(unittest unit_test/test.cpp unit_test/test-protocol.cpp ${COMMON_SOURCES})
target_include_directories(unittest PRIVATE googletest/include)
target_link_directories(unittest PRIVATE ${PROJECT_SOURCE_DIR}/googletest/build)
target_link_libraries(unittest PRIVATE pthread ${GTEST_LIBS})
