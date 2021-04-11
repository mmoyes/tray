cmake_minimum_required(VERSION 3.0.0)
project(tray VERSION 0.1.0)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(CTest)
enable_testing()



add_executable(example src/example.cpp)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon.ico
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(example 
    shell32
    gdi32 
)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
