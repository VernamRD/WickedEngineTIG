include_guard(GLOBAL)

function(project_add_module)
    set(options "")
    set(oneValueArgs MODULE_NAME MODULE_PATH)
    set(multiValueArgs "")

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT TARGET ${ARG_MODULE_NAME})
        if(NOT ARG_MODULE_PATH)
            message(FATAL_ERROR "Module ${ARG_MODULE_PATH} not found and PATH not provided!")
        endif()

        message(STATUS "${PROJECT_NAME}: [Module] Adding: ${ARG_MODULE_NAME} from ${ARG_MODULE_PATH}")
        add_subdirectory("${ARG_MODULE_PATH}/${ARG_MODULE_NAME}")
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE ${ARG_MODULE_NAME})
endfunction()

function(project_add_modules)
    set(options "")
    set(oneValueArgs  MODULE_PATH)
    set(multiValueArgs MODULE_NAMES)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(MODULE_NAME IN LISTS ARG_MODULE_NAMES)
        project_add_module(
            MODULE_NAME ${MODULE_NAME}
            MODULE_PATH "${ARG_MODULE_PATH}"
        )
    endforeach()
endfunction()