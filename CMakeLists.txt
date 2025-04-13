cmake_minimum_required(VERSION 3.25)
project(Vkxel)

# Ensure the compiler is MSVC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "${PROJECT_NAME} requires the MSVC toolchain. Current toolchain is ${CMAKE_CXX_COMPILER_ID}")
endif ()

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXTERNAL_PATH "external")

set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_CMAKE_PATH "vcpkg/scripts/buildsystems/vcpkg.cmake")
include("${EXTERNAL_PATH}/${VCPKG_CMAKE_PATH}")

set(RUNTIME_OUTPUT_PATH "bin")
set(LIBRARY_OUTPUT_PATH "lib")
set(ARCHIVE_OUTPUT_PATH "arc")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${RUNTIME_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${LIBRARY_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${ARCHIVE_OUTPUT_PATH})

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
        ${EXTERNAL_PATH}/${VULKAN_MEMORY_ALLOCATOR_PATH}/src/VmaUsage.h
        ${EXTERNAL_PATH}/${VULKAN_MEMORY_ALLOCATOR_PATH}/src/VmaUsage.cpp
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
set(SLANG_ENABLE_TESTS OFF)
set(SLANG_ENABLE_EXAMPLES OFF)
add_subdirectory(${EXTERNAL_PATH}/${SLANG_PATH})
# https://github.com/shader-slang/slang/issues/5871
add_dependencies(slang slang-glslang)
# https://github.com/shader-slang/slang/issues/5896
set_target_properties(slang slang-glslang PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${RUNTIME_OUTPUT_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${RUNTIME_OUTPUT_PATH}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${ARCHIVE_OUTPUT_PATH}
)

# imgui
set(IMGUI_PATH "imgui")
set(IMGUI_INCLUDE_DIR
        ${EXTERNAL_PATH}/${IMGUI_PATH}
        ${EXTERNAL_PATH}/${IMGUI_PATH}/backends
)
set(IMGUI_SOURCE_FILES
        ${EXTERNAL_PATH}/${IMGUI_PATH}/imgui.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/imgui_demo.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/imgui_draw.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/imgui_tables.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/imgui_widgets.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
        ${EXTERNAL_PATH}/${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
)

# reflect-cpp
set(REFLECT_CPP_PATH "reflect-cpp")
add_subdirectory(${EXTERNAL_PATH}/${REFLECT_CPP_PATH})

# nameof
set(NAMEOF_PATH "nameof")
add_subdirectory(${EXTERNAL_PATH}/${NAMEOF_PATH})

# EnTT
set(ENTT_PATH "entt")
add_subdirectory(${EXTERNAL_PATH}/${ENTT_PATH})

# spdlog
set(SPDLOG_PATH "spdlog")
add_subdirectory(${EXTERNAL_PATH}/${SPDLOG_PATH})

function(VKXEL_DEFINE_SOURCES OUT_LIST BASE_PATH)
    set(FILES "${ARGN}")
    list(TRANSFORM FILES PREPEND ${BASE_PATH}/)
    set(${OUT_LIST} "${FILES}" PARENT_SCOPE)
endfunction()


VKXEL_DEFINE_SOURCES(ENGINE_SOURCES "engine"
        window.cpp
        window.h
        renderer.cpp
        renderer.h
        shader.cpp
        shader.h
        file.cpp
        file.h
        data_type.h
        input.cpp
        input.h
        vtime.cpp
        vtime.h
        gui.cpp
        gui.h
        resource_type.h
        resource.cpp
        resource.h
        timer.cpp
        timer.h
        engine.cpp
        engine.h
        compute.cpp
        compute.h
)

VKXEL_DEFINE_SOURCES(EDITOR_SOURCES "editor"
        editor.cpp
        editor.h
)

VKXEL_DEFINE_SOURCES(WORLD_SOURCES "world"
        camera.cpp
        camera.h
        transform.cpp
        transform.h
        controller.cpp
        controller.h
        object.h
        gameobject.hpp
        component.h
        mesh.cpp
        mesh.h
        drawer.cpp
        drawer.h
        mover.cpp
        mover.h
        canvas.cpp
        canvas.h
        scene.cpp
        scene.h
)

VKXEL_DEFINE_SOURCES(VKUTIL_SOURCES "vkutil"
        image.cpp
        image.h
        buffer.cpp
        buffer.h
        command.cpp
        command.h
        descriptor.cpp
        descriptor.h
        pipeline.cpp
        pipeline.h
)

VKXEL_DEFINE_SOURCES(UTIL_SOURCES "util"
        application.h
        check.h
        delegate.hpp
        debug.hpp
)

VKXEL_DEFINE_SOURCES(REFLECT_SOURCES "reflect"
        reflect.cpp
        reflect.hpp
)

VKXEL_DEFINE_SOURCES(ENTRY_SOURCES "entry"
        model_library.cpp
        model_library.h
        scene_library.cpp
        scene_library.h
        main.cpp
)

VKXEL_DEFINE_SOURCES(CUSTOM_SOURCES "custom"
        sdf_surface.cpp
        sdf_surface.h
        dual_contouring.cpp
        dual_contouring.h
        gpu_dual_contouring.cpp
        gpu_dual_contouring.h
)

set(VKXEL_SOURCE_PATH "source")
VKXEL_DEFINE_SOURCES(VKXEL_SOURCES ${VKXEL_SOURCE_PATH}
        ${ENGINE_SOURCES}
        ${EDITOR_SOURCES}
        ${WORLD_SOURCES}
        ${VKUTIL_SOURCES}
        ${UTIL_SOURCES}
        ${REFLECT_SOURCES}
        ${ENTRY_SOURCES}
        ${CUSTOM_SOURCES}
)

# Set the source files
set(SOURCES
        ${VMA_USAGE_FILES}
        ${IMGUI_SOURCE_FILES}
        ${VKXEL_SOURCES}
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
include_directories(
        ${Vulkan_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/${VKXEL_SOURCE_PATH}
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
        reflectcpp
        nameof
        EnTT::EnTT
        spdlog::spdlog
)

# Enable all warnings and new preprocessor during compile
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:preprocessor)

# Add RELEASE macro in Release
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:RELEASE>)

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
