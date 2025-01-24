//
// Created by jiayi on 1/19/2025.
//

#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <utility>

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

#include "shader.h"
#include "check.h"
#include "file.h"

namespace Vkxel {
    ShaderLoader &ShaderLoader::Instance() {
        static ShaderLoader instance;
        return instance;
    }

    ShaderLoader::ShaderLoader() {
        createGlobalSession(_slang_global_session.writeRef());
        CHECK_NOTNULL(_slang_global_session);

        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = _slang_global_session->findProfile("spirv_1_6");

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        std::array options = {
            slang::CompilerOptionEntry{
                slang::CompilerOptionName::VulkanUseEntryPointName,
                {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            },
            slang::CompilerOptionEntry{
                slang::CompilerOptionName::GenerateWholeProgram,
                {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            }
        };
        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());

        _slang_global_session->createSession(sessionDesc, _slang_session.writeRef());

        CHECK_NOTNULL(_slang_session);
    }

    std::vector<uint8_t> ShaderLoader::LoadToBinary(std::string_view shader, bool force_compile) {
        std::string full_path = _shader_resource_folder;
        full_path += shader;
        std::string spirv_full_path = full_path + _spirv_extension;
        std::string slang_full_path = full_path + _slang_extension;
        bool exist_spirv = File::Exist(spirv_full_path);
        bool exist_slang = File::Exist(slang_full_path);
        CHECK_NOTNULL_MSG(exist_slang, "Shader Not Exist: " << slang_full_path);
        if (exist_spirv && !force_compile) {
            return LoadSpirv(spirv_full_path);
        }
        std::vector<uint8_t> spirv = LoadSlang(slang_full_path);
        File::WriteBinaryFile(spirv_full_path, spirv);
        return spirv;
    }

    VkShaderModule ShaderLoader::LoadToModule(VkDevice device, std::string_view shader, bool force_compile) {
        std::vector<uint8_t> shader_code = LoadToBinary(shader, force_compile);
        VkShaderModuleCreateInfo shader_module_create_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = shader_code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(shader_code.data())
        };
        VkShaderModule shader_module;
        CHECK_RESULT_VK(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &shader_module));
        return shader_module;
    }


    void ShaderLoader::ClearSpirvCache() {
        for (const auto& entry : std::filesystem::directory_iterator(_shader_resource_folder)) {
            if (entry.is_regular_file() && entry.path().extension() == _spirv_extension) {
                std::filesystem::remove(entry.path());
            }
        }
    }

    void ShaderLoader::SetResourceFolder(const std::string_view resource_folder) {
        _shader_resource_folder = resource_folder;
    }


    std::vector<uint8_t> ShaderLoader::LoadSlang(const std::string_view shader_file) {

        Slang::ComPtr<slang::IModule> module;
        {
            Slang::ComPtr<slang::IBlob> diagnosticBlob;
            module = _slang_session->loadModule(shader_file.data(), diagnosticBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticBlob.get(), static_cast<const char*>(diagnosticBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(module);
        }

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticBlob;
            module->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticBlob.get(), static_cast<const char*>(diagnosticBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(linkedProgram);
        }

        Slang::ComPtr<slang::IBlob> code;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getTargetCode(
                0,
                code.writeRef(),
                diagnosticsBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticsBlob.get(), static_cast<const char*>(diagnosticsBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(!result);
        }

        CHECK_NOTNULL(code->getBufferSize());
        std::vector<uint8_t> spirv(code->getBufferSize());
        std::copy_n(static_cast<const uint8_t*>(code->getBufferPointer()), code->getBufferSize(), spirv.data());
        return spirv;
    }

    std::vector<uint8_t> ShaderLoader::LoadSpirv(const std::string_view shader_file) {
        return File::ReadBinaryFile(shader_file);
    }

} // Vkxel