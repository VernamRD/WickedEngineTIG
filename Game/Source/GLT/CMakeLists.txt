project(GLT LANGUAGES CXX)

# Module sources
file(GLOB SOURCES
    source/*.cpp
    include/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PUBLIC ${WICKED_ROOT_DIR}  # доступ к WickedEngine
)

target_link_libraries(${PROJECT_NAME} PUBLIC WickedEngine)

# Link modules
project_add_module(
    MODULE_NAME Core
    MODULE_PATH "${GAME_ROOT_DIR}/Source"
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()
