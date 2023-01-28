cmake_minimum_required(VERSION 3.10)
project(msg-lite CXX)
set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES src/log.cpp)

add_executable(ms-lite ms-lite.cpp src/server_application.cpp ${COMMON_SOURCES})
add_executable(mc-lite mc-lite.cpp  ${COMMON_SOURCES})

target_include_directories(ms-lite PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/spdlog/include)
target_include_directories(mc-lite PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/spdlog/include)

target_link_libraries(ms-lite PRIVATE pthread ${PROJECT_SOURCE_DIR}/spdlog/build/libspdlog.a)
target_link_libraries(mc-lite PRIVATE ${PROJECT_SOURCE_DIR}/spdlog/build/libspdlog.a)
