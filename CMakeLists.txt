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

IF (WIN32)
    target_link_libraries(example shell32 gdi32 )
ELSEIF(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_search_module (appindicator REQUIRED IMPORTED_TARGET appindicator3-0.1)
    target_include_directories(example PUBLIC ${GTK3_INCLUDE_DIRS})
    target_link_libraries(example ${GTK3_LIBRARY_DIRS} PkgConfig::appindicator )
ELSEIF( APPLE )
ENDIF()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
