//
// Created by jiayi on 1/20/2025.
//

#ifndef VKXEL_FILE_H
#define VKXEL_FILE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Vkxel {

    class File {
    public:
        File() = delete;
        ~File() = delete;

        static bool Exist(std::string_view filePath);

        static std::string ReadTextFile(std::string_view filePath);
        static std::vector<uint8_t> ReadBinaryFile(std::string_view filePath);

        static void WriteTextFile(std::string_view filePath, std::string_view fileContent);
        static void WriteBinaryFile(std::string_view filePath, const std::vector<uint8_t> &fileContent);
    };

} // namespace Vkxel

#endif // VKXEL_FILE_H
