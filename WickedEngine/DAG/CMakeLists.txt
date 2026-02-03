project(DAG)

# Module sources
file(GLOB SOURCES
    source/*.cpp
    include/*.h
    source/Tasks/*.h
    source/Tasks/*.cpp
)

# if(BUILD_TESTING)
#     add_subdirectory(tests)
# endif()

set(PROJECT_OBJECTS ${PROJECT_NAME}Objects)

add_library(DAGObject OBJECT ${SOURCES})

target_include_directories(DAGObject
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
    $<TARGET_PROPERTY:WickedEngine_common,INTERFACE_INCLUDE_DIRECTORIES>
)