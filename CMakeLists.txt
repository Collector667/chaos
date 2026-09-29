cmake_minimum_required(VERSION 3.28)
project(num_diff_methods CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. raylib
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# 2. Eigen (скачиваем исходники без запуска лишних скриптов)
FetchContent_Declare(
        eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG        3.4.0
)
FetchContent_GetProperties(eigen)
if(NOT eigen_POPULATED)
    FetchContent_Populate(eigen)
    add_library(Eigen INTERFACE)
    target_include_directories(Eigen INTERFACE ${eigen_SOURCE_DIR})
endif()

# 3. Исполняемый файл приложения
add_executable(untitled main.cpp)

# 4. Регистрация C++20 модулей
target_sources(untitled
        PUBLIC
        FILE_SET CXX_MODULES FILES
        methods/CDMethod.ixx
        methods/EulerMethod.ixx
        methods/EulerCramerMethod.ixx
        methods/MidPointMethod.ixx
        methods/RungeKuttaMethods.ixx
        funcs/csv.ixx
        funcs/chaosRisuyka.ixx
)

# 5. Пути к заголовкам самого проекта
target_include_directories(untitled PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 6. Линковка библиотек
target_link_libraries(untitled PRIVATE raylib Eigen)