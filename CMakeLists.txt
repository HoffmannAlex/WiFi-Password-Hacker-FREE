cmake_minimum_required(VERSION 3.10)

# Project settings
project(HackWifiAI VERSION 1.0.0
        DESCRIPTION "AI-Powered WiFi Security Testing Tool"
        LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Find required packages
find_package(Threads REQUIRED)

# Enable testing
enable_testing()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the main library
add_library(hack_wifi_ai
    src/WiFiNetwork.cpp
    src/WiFiScanner.cpp
    src/HandshakeCapturer.cpp
    src/DeauthAttacker.cpp
    src/AIPasswordGenerator.cpp
    src/AICrackEngine.cpp
    src/SecurityAnalyzer.cpp
    src/HackWifiAI.cpp
)

target_include_directories(hack_wifi_ai
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hack_wifi_ai
    PRIVATE
        Threads::Threads
        ${CMAKE_THREAD_LIBS_INIT}
)

# Add tests subdirectory
add_subdirectory(tests)

# Add main executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# Set target properties
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hack_wifi_ai)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)  # Windows 10
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
else()
    # Unix/Linux-specific settings
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
    
    # Add compile flags for better warnings
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic -Werror
        -Wno-unused-parameter -Wno-missing-field-initializers
    )
    
    # Add hardening flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=2
        -fPIE
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,-z,now
        -Wl,-z,relro
        -pie
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Add uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()

# Add test target
enable_testing()
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add documentation target
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    
    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    
    doxygen_add_docs(docs
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        COMMENT "Generating API documentation with Doxygen"
    )
endif()
