cmake_minimum_required(VERSION 3.25)
project(Vkxel)

# Ensure the compiler is MSVC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "${PROJECT_NAME} requires the MSVC toolchain. Current toolchain is ${CMAKE_CXX_COMPILER_ID}")
endif()

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBRARY_OUTPUT_PATH "lib")
set(RUNTIME_OUTPUT_PATH "bin")
set(ARCHIVE_OUTPUT_PATH "arc")

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
set(VMA_USAGE_FILES
        "${EXTERNAL_PATH}/${VULKAN_MEMORY_ALLOCATOR_PATH}/src/VmaUsage.h"
        "${EXTERNAL_PATH}/${VULKAN_MEMORY_ALLOCATOR_PATH}/src/VmaUsage.cpp"
)

# glm
set(GLM_PATH "glm")
add_subdirectory(${EXTERNAL_PATH}/${GLM_PATH})

# slang
set(SLANG_PATH "slang")
set(SLANG_USE_SYSTEM_VULKAN_HEADERS ON)
set(SLANG_EMBED_CORE_MODULE_SOURCE OFF)
set(SLANG_ENABLE_SLANG_RHI OFF)
set(SLANG_ENABLE_DXIL OFF)
set(SLANG_ENABLE_GFX OFF)
set(SLANG_ENABLE_SLANGD OFF)
set(SLANG_ENABLE_SLANGC OFF)
set(SLANG_ENABLE_SLANGRT OFF)
set(SLANG_ENABLE_SLANG_GLSLANG OFF)
set(SLANG_ENABLE_TESTS OFF)
set(SLANG_ENABLE_EXAMPLES OFF)
add_subdirectory(${EXTERNAL_PATH}/${SLANG_PATH})
set_target_properties(slang PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${RUNTIME_OUTPUT_PATH}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${ARCHIVE_OUTPUT_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${LIBRARY_OUTPUT_PATH}
)

# Set the source files
set(SOURCES
        ${VMA_USAGE_FILES}
        source/main.cpp
        source/window.cpp
        source/window.h
        source/check.h
        source/renderer.cpp
        source/renderer.h
        source/application.h
        source/shader.cpp
        source/shader.h
        source/file.cpp
        source/file.h
        source/model.cpp
        source/model.h
        source/camera.cpp
        source/camera.h
        source/transform.cpp
        source/transform.h
        source/buffer.h
        source/input.cpp
        source/input.h
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
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${RUNTIME_OUTPUT_PATH}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${ARCHIVE_OUTPUT_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${LIBRARY_OUTPUT_PATH}
)

# Copy shader folder to the binary directory after building
set(SOURCE_SHADER_PATH "shader")
set(TARGET_SHADER_PATH "shader")
add_custom_target(${PROJECT_NAME}_Shader ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TARGET_SHADER_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${SOURCE_SHADER_PATH}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TARGET_SHADER_PATH}
        COMMENT "Copying shader folder to binary directory."
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_Shader)

# Group source files in folders (optional, for IDEs like Visual Studio)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES})

enable_testing()
set(CTEST_NAME_PREFIX "${PROJECT_NAME}_Test")
add_test(NAME ${CTEST_NAME_PREFIX}_Dummy COMMAND ${CMAKE_COMMAND} -E echo "${PROJECT_NAME} Dummy Test")
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES TIMEOUT 60)
set_tests_properties(${CTEST_NAME_PREFIX}_Dummy PROPERTIES FAIL_REGULAR_EXPRESSION "Error|Failed")
