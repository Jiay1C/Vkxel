//
// Created by jiayi on 1/20/2025.
//

#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "file.h"
#include "util/check.h"

namespace Vkxel {
    bool File::Exist(std::string_view filePath) {
        std::ifstream file(filePath.data());
        return file.good();
    }

    std::string File::ReadTextFile(std::string_view filePath) {
        std::ifstream file(filePath.data(), std::ios::in | std::ios::binary);
        CHECK_NOTNULL_MSG(file.good(), "Unable To Read Text File " << filePath);

        file.seekg(0, std::ios_base::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios_base::beg);

        std::string buffer;
        buffer.resize(fileSize);

        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    std::vector<uint8_t> File::ReadBinaryFile(std::string_view filePath) {
        std::ifstream file(filePath.data(), std::ios::in | std::ios::binary);
        CHECK_NOTNULL_MSG(file.good(), "Unable To Read Binary File " << filePath);

        file.seekg(0, std::ios_base::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios_base::beg);

        std::vector<uint8_t> buffer(fileSize);
        file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
        file.close();

        return buffer;
    }

    void File::WriteTextFile(std::string_view filePath, std::string_view fileContent) {
        std::ofstream file(filePath.data(), std::ios::out | std::ios::trunc | std::ios::binary);
        CHECK_NOTNULL_MSG(file.good(), "Unable To Write Text File " << filePath);

        file.write(fileContent.data(), fileContent.size());
        file.close();
    }

    void File::WriteBinaryFile(std::string_view filePath, const std::vector<uint8_t> &fileContent) {
        std::ofstream file(filePath.data(), std::ios::out | std::ios::trunc | std::ios::binary);
        CHECK_NOTNULL_MSG(file.good(), "Unable To Write Binary File " << filePath);

        file.write(reinterpret_cast<const char *>(fileContent.data()), fileContent.size());
        file.close();
    }
} // namespace Vkxel
