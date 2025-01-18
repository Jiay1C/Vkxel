cmake_minimum_required(VERSION 3.20)
project(Vkxel)

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXTERNAL_PATH "external")

set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_CMAKE_PATH "${EXTERNAL_PATH}/vcpkg/scripts/buildsystems/vcpkg.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/${VCPKG_CMAKE_PATH}")

# GLFW
find_package(glfw3 3.4 REQUIRED)

# Vulkan
set(VULKAN_HEADERS_PATH "${EXTERNAL_PATH}/Vulkan-Headers")
add_subdirectory(${VULKAN_HEADERS_PATH})

set(VULKAN_LOADER_PATH "${EXTERNAL_PATH}/Vulkan-Loader")
add_subdirectory(${VULKAN_LOADER_PATH})

set(VULKAN_UTILITY_PATH "${EXTERNAL_PATH}/Vulkan-Utility-Libraries")
add_subdirectory(${VULKAN_UTILITY_PATH})

# vk-bootstrap
set(VK_BOOTSTRAP_PATH "${EXTERNAL_PATH}/vk-bootstrap")
add_subdirectory(${VK_BOOTSTRAP_PATH})

# VulkanMemoryAllocator
set(VULKAN_MEMORY_ALLOCATOR_PATH "${EXTERNAL_PATH}/VulkanMemoryAllocator")
add_subdirectory(${VULKAN_MEMORY_ALLOCATOR_PATH})

# glm
set(GLM_PATH "${EXTERNAL_PATH}/glm")
add_subdirectory(${GLM_PATH})

# Set the source files
set(SOURCES
        source/main.cpp
        source/window.cpp
        source/window.h
        source/check.h
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
        glfw
        Vulkan::Headers
        Vulkan::Loader
        Vulkan::UtilityHeaders
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        glm::glm
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
set(CTEST_NAME_PREFIX "${PROJECT_NAME}_Test")
add_test(NAME ${CTEST_NAME_PREFIX}_Dummy COMMAND ${CMAKE_COMMAND} -E echo "${PROJECT_NAME} Dummy Test")
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES TIMEOUT 60)
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES FAIL_REGULAR_EXPRESSION "Error|Failed")