# Копирует сгенерированные модульные .ifc из каталога сборки в отдельную папку,
# из которой их читает IntelliSense (C/C++, cpptools).
#
# Зачем копия, а не оригинал: cpptools открывает .ifc по флагу /reference и
# удерживает файл открытым. Если IntelliSense читает оригинал из
# build/untitled.dir/Debug, то при следующей сборке изменённого модуля MSVC
# не может перезаписать свой же IFC и сборка падает с
#   error C3474: could not open output file '...ifc'
# Копия снимает эту проблему: компилятор пишет оригинал, IntelliSense читает копию.
#
# Ошибки копирования (файл может быть занят IntelliSense) намеренно НЕ валят
# сборку — в этом случае просто сообщаем, что копия не обновилась.
#
# Сообщение печатается через message(STATUS), а не message(WARNING): строки
# "CMake Warning ... at ...:31" подсвечиваются в панели Problems VS Code как
# проблемы задачи сборки, а это не проблема сборки, а нормальный симптом
# "cpptools держит .ifc открытым". STATUS виден в логе сборки и Problems не мусорит.
# Лечится перезагрузкой окна VS Code (Ctrl+Shift+P → Developer: Reload Window)
# и повторной сборкой: после перезагрузки cpptools отпускает .ifc.
#
# Запускается из CMakeLists.txt как POST_BUILD шаг. Параметры:
#   IFC_SRC_DIR — каталог с .ifc после сборки
#   IFC_DST_DIR — каталог с копиями для IntelliSense

file(GLOB ifc_files "${IFC_SRC_DIR}/*.ifc")
file(MAKE_DIRECTORY "${IFC_DST_DIR}")

foreach(ifc_file IN LISTS ifc_files)
    get_filename_component(ifc_name "${ifc_file}" NAME)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${ifc_file}" "${IFC_DST_DIR}/${ifc_name}"
        RESULT_VARIABLE copy_result
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(NOT copy_result EQUAL 0)
        message(STATUS
                "IntelliSense: НЕ обновилась копия ${ifc_name} "
                "(файл занят, обычно cpptools-srv.exe). Модуль в IntelliSense будет "
                "показан в старой версии, пока не перезагрузите окно VS Code "
                "(Developer: Reload Window) и не соберёте снова.")
    endif()
endforeach()

list(LENGTH ifc_files ifc_count)
message(STATUS "IntelliSense: .ifc для копирования — ${ifc_count}, папка ${IFC_DST_DIR}")
