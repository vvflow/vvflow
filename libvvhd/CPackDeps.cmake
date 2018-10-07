cmake_minimum_required (VERSION 3.0)

function(add_dependency lib_list)
    list(GET lib_list 0 lib_name)
    execute_process(
        COMMAND readlink -f "${lib_name}"
        OUTPUT_VARIABLE lib_path
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
        COMMAND dpkg -S "${lib_path}"
        COMMAND cut -d: -f1
        OUTPUT_VARIABLE lib_package
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND DEPENDENCIES "${lib_package}")
    set(DEPENDENCIES ${DEPENDENCIES} PARENT_SCOPE)
endfunction(add_dependency)
