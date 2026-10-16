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
# raylib 5.5 объявляет cmake_minimum_required(VERSION 3.5), поэтому CMake 4
# предупреждает: "Compatibility with CMake < 3.10 will be removed...". Поднимаем
# минимальную версию политик для подпроекта, не правя чужой код — это
# документированный способ (cmake --help-variable CMAKE_POLICY_VERSION_MINIMUM:
# "Projects may set this variable before a call to add_subdirectory() that adds a
# third-party project"). 3.10 — самое низкое значение, снимающее предупреждение,
# т.е. меньше всего меняет поведение raylib.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(raylib)

# 2. Eigen (нужны только заголовки: исходники скачиваем, но CMake-скрипты Eigen'а
#    в сборку не подключаем). SOURCE_SUBDIR указывает на несуществующий подкаталог,
#    и тогда FetchContent_MakeAvailable() только выкачивает содержимое, не вызывая
#    add_subdirectory() — это штатный приём, см. комментарий в FetchContent.cmake
#    ("Protecting the call with the check allows this function to be used for
#    projects that just want to ensure the content exists").
FetchContent_Declare(
        eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG        3.4.0
        SOURCE_SUBDIR  not-a-subproject
)
FetchContent_MakeAvailable(eigen)

add_library(Eigen INTERFACE)
target_include_directories(Eigen INTERFACE ${eigen_SOURCE_DIR})

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
target_compile_definitions(untitled PRIVATE
        PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# 7. Копии модульных .ifc для IntelliSense
# Расширение C/C++ (cpptools) читает модули по флагам /reference из
# .vscode/c_cpp_properties.json и удерживает .ifc открытым. Поэтому IntelliSense
# не должен ссылаться на оригиналы из каталога сборки: иначе пересборка модуля
# падала бы с "error C3474: could not open output file". Копируем .ifc в
# сборку не останавливают).
#
# Шаг имеет смысл только для MSVC с генератором Visual Studio: и формат .ifc,
# и раскладка каталогов "untitled.dir/<CONFIG>" — особенности именно этой связки
# (на них и рассчитаны флаги /reference в .vscode/c_cpp_properties.json).
# У clang (macOS/Linux) модули компилируются в .pcm и IntelliSense работает
# через clangd/compile_commands.json, а на Ninja-сборках MSVC путь к модулям
# другой — копировать было бы попросту нечего.
if(MSVC AND CMAKE_GENERATOR MATCHES "Visual Studio")
    add_custom_command(
            TARGET untitled POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                    -DIFC_SRC_DIR=${CMAKE_CURRENT_BINARY_DIR}/untitled.dir/$<CONFIG>
                    -DIFC_DST_DIR=${CMAKE_BINARY_DIR}/ifc-intellisense
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ifc_for_intellisense.cmake
            COMMENT "Обновляю .ifc для IntelliSense"
    )
endif()
