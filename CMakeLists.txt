cmake_minimum_required(VERSION 3.16)

project(CoAPClient LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COAP_ENABLE_WARNINGS "Enable extra compiler warnings" ON)

set(_warning_flags -Wall -Wextra -Wpedantic)

add_executable(coap_client
    src/main.cpp
    src/message.cpp
    src/client.cpp
    src/option.cpp
    src/cli.cpp
    src/metrics.cpp
    src/metrics_server.cpp
    src/mqtt_publisher.cpp
)
if(COAP_ENABLE_WARNINGS)
    target_compile_options(coap_client PRIVATE ${_warning_flags})
endif()

target_include_directories(coap_client PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link pthread for threading support
find_package(Threads REQUIRED)
target_link_libraries(coap_client PRIVATE Threads::Threads)

# Link Paho MQTT C library
target_link_libraries(coap_client PRIVATE paho-mqtt3c)

