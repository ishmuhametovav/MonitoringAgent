cmake_minimum_required(VERSION 4.3)
project(MonitoringAgent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00)
endif ()

include(FetchContent)

set(HTTPLIB_USE_NON_BLOCKING_GETADDRINFO OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.58.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(httplib nlohmann_json)

add_executable(${PROJECT_NAME}
        main.cpp
        metrics/MetricsProvider.h
        queue/MessageQueue.cpp
        queue/MessageQueue.h
        packet/packet.h
        packet/packet.cpp
        Application.cpp
        Application.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        httplib nlohmann_json
)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
            metrics/WindowsMetricsProvider.cpp
            metrics/WindowsMetricsProvider.h)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    find_library(XSS_LIB NAMES Xss REQUIRED)
    target_sources(${PROJECT_NAME} PRIVATE
            metrics/X11MetricsProvider.cpp
            metrics/X11MetricsProvider.h)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11 ${XSS_LIB})
endif ()

add_executable(Server TestServer.cpp)
target_link_libraries(Server PRIVATE httplib nlohmann_json)
