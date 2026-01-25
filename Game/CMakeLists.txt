project(Game)

# Module library
set(PROJECT_MODULES
    GLT
    Core
    GPUCPI
)

# Enable testing
option(BUILD_TESTING "Build the testing tree" ON)

if(BUILD_TESTING)
    # Connect GTest
    if(NOT TARGET GTest::gtest_main)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
        )
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
        target_link_options(${PROJECT_NAME} PRIVATE --coverage)
    endif()

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # Global test agregator
    add_custom_target(AllGameTests)
endif()

# Add module subdirectories
foreach(MODULE_NAME IN LISTS PROJECT_MODULES)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Source/${MODULE_NAME}")
endforeach()

file(GLOB SOURCE_FILES
    "Source/*.cpp"
    "Source/*.h"
)

set(LIB_DXCOMPILER "dxcompiler.dll")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if(BUILD_TESTING AND TARGET AllGameTests)
    add_dependencies(${PROJECT_NAME} AllGameTests)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "${WICKED_ROOT_DIR}/WickedEngine/WickedEngine.h")

target_link_libraries(${PROJECT_NAME} PUBLIC
    WickedEngine
)

# Link modules
foreach(MODULE_NAME IN LISTS PROJECT_MODULES)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${MODULE_NAME}
    )
endforeach()

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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBDXCOMPILER_PATH} ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${STARTUP_LUA} ${CMAKE_CURRENT_BINARY_DIR}/startup.lua
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Content"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Content"
    COMMENT "Copying Content folder..."
)