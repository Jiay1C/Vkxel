cmake_minimum_required(VERSION 3.20)
project(Vkxel)

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_CMAKE_PATH "external/vcpkg/scripts/buildsystems/vcpkg.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/${VCPKG_CMAKE_PATH}")

# Find Vulkan
find_package(Vulkan 1.3 REQUIRED)

# Find GLFW
find_package(glfw3 3.4 REQUIRED)

# Set the source files
set(SOURCES
        source/main.cpp
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
include_directories(
        ${Vulkan_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
# Link Vulkan, GLFW, and other necessary libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
)

# Output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/lib
)

# Group source files in folders (optional, for IDEs like Visual Studio)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES})

enable_testing()
set(CTEST_NAME_PREFIX "${PROJECT_NAME}_TEST")
add_test(NAME ${CTEST_NAME_PREFIX}_Dummy COMMAND ${CMAKE_COMMAND} -E echo "${PROJECT_NAME} Dummy Test")
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES TIMEOUT 60)
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES FAIL_REGULAR_EXPRESSION "Error|Failed")