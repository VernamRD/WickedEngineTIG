project(DAGTests)

add_executable(DAGTests
        DAGTests.cpp)

target_link_libraries(${PROJECT_NAME} 
        PRIVATE 
        GTest::gmock_main 
        DAGObject
        WickedEngine
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ../source
    ../include
)

# Register test
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

if(WIN32)
    set(LIB_DXCOMPILER "dxcompiler.dll")
else()
    set(LIB_DXCOMPILER "libdxcompiler.so")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD

    # Copy dxcompiler library
    COMMAND ${CMAKE_COMMAND} -E echo " Copying shader compile library ${WICKED_ROOT_DIR}/WickedEngine/${LIB_DXCOMPILER}..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WICKED_ROOT_DIR}/WickedEngine/${LIB_DXCOMPILER}"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E echo " Copying shader compile library complete!"

    # Copy shaders
    COMMAND ${CMAKE_COMMAND} -E echo " Copying shaders from ${CMAKE_CURRENT_SOURCE_DIR}/shaders..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMAND ${CMAKE_COMMAND} -E echo " Copying shaders complete!"

    VERBATIM
)

