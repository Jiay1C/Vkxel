//
// Created by jiayi on 1/19/2025.
//

#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

#include "shader.h"
#include "check.h"
#include "file.h"

namespace Vkxel {
    ShaderLoader::ShaderLoader() {
        createGlobalSession(_slang_global_session.writeRef());
        CHECK_NOTNULL(_slang_global_session);
    }

    ShaderLoader &ShaderLoader::Instance() {
        static ShaderLoader instance;
        return instance;
    }

    std::vector<uint8_t> ShaderLoader::Load(std::string_view shader, bool force_compile) {
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

        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = _slang_global_session->findProfile("spirv_1_5");

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        std::array<slang::CompilerOptionEntry, 1> options =
            {
            {
                slang::CompilerOptionName::EmitSpirvDirectly,
                {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            }
            };
        sessionDesc.compilerOptionEntries = options.data();
        sessionDesc.compilerOptionEntryCount = options.size();

        Slang::ComPtr<slang::ISession> session;
        _slang_global_session->createSession(sessionDesc, session.writeRef());

        Slang::ComPtr<slang::IModule> slangModule;
        {
            Slang::ComPtr<slang::IBlob> diagnosticBlob;
            slangModule = session->loadModule(shader_file.data(), diagnosticBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticBlob.get(), static_cast<const char*>(diagnosticBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(slangModule);
        }

        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            slangModule->findEntryPointByName("main", entryPoint.writeRef());
            CHECK_NOTNULL(entryPoint);
        }

        std::array<slang::IComponentType*, 2> componentTypes =
        {
            slangModule,
            entryPoint
        };

        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticsBlob.get(), static_cast<const char*>(diagnosticsBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(!result);
        }

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = composedProgram->link(
                linkedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticsBlob.get(), static_cast<const char*>(diagnosticsBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(!result);
        }


        Slang::ComPtr<slang::IBlob> spirvCode;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                0,
                0,
                spirvCode.writeRef(),
                diagnosticsBlob.writeRef());
            CHECK_NOTNULL_MSG(!diagnosticsBlob.get(), static_cast<const char*>(diagnosticsBlob.get()->getBufferPointer()));
            CHECK_NOTNULL(!result);
        }

        CHECK_NOTNULL(spirvCode->getBufferSize());
        std::vector<uint8_t> spirv(spirvCode->getBufferSize());
        std::copy_n(static_cast<const uint8_t*>(spirvCode->getBufferPointer()), spirvCode->getBufferSize(), spirv.data());
        return spirv;
    }

    std::vector<uint8_t> ShaderLoader::LoadSpirv(const std::string_view shader_file) {
        return File::ReadBinaryFile(shader_file);
    }

} // Vkxel