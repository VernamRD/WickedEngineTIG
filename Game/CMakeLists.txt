project(Game)

set(GAME_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCE_FILES
    "Source/*.cpp"
    "Source/*.h"
)

if(WIN32)
    set(LIB_DXCOMPILER "dxcompiler.dll")
elseif(UNIX)
    set(LIB_DXCOMPILER "libdxcompiler.so")
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

project_add_modules(
    MODULE_NAMES Core GLT
    MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${WICKED_ROOT_DIR}/WickedEngine/WickedEngine.h")

target_link_libraries(${PROJECT_NAME} PUBLIC
    WickedEngine
)

if(MSVC)
    add_compile_options(/MP)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Game PRIVATE /bigobj /Zi)
    target_link_options(Game PRIVATE /INCREMENTAL /DEBUG:FASTLINK)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE YES
)

if(WICKED_ENABLE_IPO)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION ON
        INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF
    )
endif()

set(LIBDXCOMPILER_PATH "${WICKED_ROOT_DIR}/WickedEngine/${LIB_DXCOMPILER}")
set(STARTUP_LUA "${WICKED_ROOT_DIR}/Editor/startup.lua")
message("libdxcompiler found at ${LIBDXCOMPILER_PATH}")
message("startup lua found at ${STARTUP_LUA}")

# NOTE: this wont work because client never has prvileges to copy files
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBDXCOMPILER_PATH} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${STARTUP_LUA} ${CMAKE_CURRENT_BINARY_DIR}/startup.lua
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Content"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Content"
    COMMENT "Copying Content folder..."
)