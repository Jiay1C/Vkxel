cmake_minimum_required(VERSION 3.20)
project(Vkxel)

# Ensure the compiler is MSVC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "${PROJECT_NAME} requires the MSVC toolchain.")
endif()

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXTERNAL_PATH "external")

set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_CMAKE_PATH "vcpkg/scripts/buildsystems/vcpkg.cmake")
include("${EXTERNAL_PATH}/${VCPKG_CMAKE_PATH}")

# GLFW
find_package(glfw3 3.4 REQUIRED)

# Vulkan
set(VULKAN_HEADERS_PATH "Vulkan-Headers")
add_subdirectory(${EXTERNAL_PATH}/${VULKAN_HEADERS_PATH})

set(VULKAN_LOADER_PATH "Vulkan-Loader")
add_subdirectory(${EXTERNAL_PATH}/${VULKAN_LOADER_PATH})

set(VULKAN_UTILITY_PATH "Vulkan-Utility-Libraries")
add_subdirectory(${EXTERNAL_PATH}/${VULKAN_UTILITY_PATH})

# vk-bootstrap
set(VK_BOOTSTRAP_PATH "vk-bootstrap")
add_subdirectory(${EXTERNAL_PATH}/${VK_BOOTSTRAP_PATH})

# VulkanMemoryAllocator
set(VULKAN_MEMORY_ALLOCATOR_PATH "VulkanMemoryAllocator")
add_subdirectory(${EXTERNAL_PATH}/${VULKAN_MEMORY_ALLOCATOR_PATH})

# glm
set(GLM_PATH "glm")
add_subdirectory(${EXTERNAL_PATH}/${GLM_PATH})

# slang
set(SLANG_PATH "slang")
set(SLANG_USE_SYSTEM_VULKAN_HEADERS ON)
add_subdirectory(${EXTERNAL_PATH}/${SLANG_PATH})

# Set the source files
set(SOURCES
        source/main.cpp
        source/window.cpp
        source/window.h
        source/check.h
        source/renderer.cpp
        source/renderer.h
        source/application.h
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
include_directories(
        ${Vulkan_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        Vulkan::Headers
        Vulkan::Loader
        Vulkan::UtilityHeaders
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        glm::glm
        slang
)

# Enable all warnings during compile
target_compile_options(${PROJECT_NAME} PRIVATE /W4)

# Add RELEASE macro in Release
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:RELEASE>)

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
