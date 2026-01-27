project(CPI LANGUAGES CXX)

# Modules
set(MODULE_DEPEND
    Core
)

# Module sources
file(GLOB SOURCES
    source/*.cpp
    include/*.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PUBLIC ${WICKED_ROOT_DIR} 
    PRIVATE source
)

target_link_libraries(${PROJECT_NAME} PUBLIC WickedEngine)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Link modules
foreach(MODULE_NAME IN LISTS MODULE_DEPEND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MODULE_NAME})
endforeach()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()
