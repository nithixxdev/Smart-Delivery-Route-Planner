cmake_minimum_required(VERSION 3.20)
project(smartroute_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG master
)
FetchContent_MakeAvailable(crow)

add_executable(smartroute
    src/main.cpp
    src/route_optimizer.cpp
)

target_include_directories(smartroute PRIVATE include)
target_link_libraries(smartroute PRIVATE Crow::Crow)

if (WIN32)
    target_compile_definitions(smartroute PRIVATE _WIN32_WINNT=0x0A00)
    target_link_libraries(smartroute PRIVATE ws2_32 mswsock)
endif()
