project(CPITests)

add_executable(${PROJECT_NAME} CPIGraphTests.cpp CPITasksTests.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gmock_main
    CPI
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ../source
    ../include
)

# Register test
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

if(TARGET AllGameTests)
    add_dependencies(AllGameTests ${PROJECT_NAME})
endif()