project(Core LANGUAGES CXX)

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

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()
