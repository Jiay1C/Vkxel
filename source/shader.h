//
// Created by jiayi on 1/19/2025.
//

#ifndef VKXEL_SHADER_H
#define VKXEL_SHADER_H

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>

#include "slang.h"
#include "slang-com-ptr.h"

namespace Vkxel {

class ShaderLoader {
public:
    static ShaderLoader& Instance();

    std::vector<uint8_t> Load(std::string_view shader, bool force_compile = false);
    void ClearSpirvCache();
    void SetResourceFolder(std::string_view resource_folder);

private:
    ShaderLoader();

    std::vector<uint8_t> LoadSlang(std::string_view shader_file);
    std::vector<uint8_t> LoadSpirv(std::string_view shader_file);

    const std::string _spirv_extension = ".spirv";
    const std::string _slang_extension = ".slang";
    std::string _shader_resource_folder = "./shader/";

    Slang::ComPtr<slang::IGlobalSession> _slang_global_session;
};

} // Vkxel

#endif //VKXEL_SHADER_H
