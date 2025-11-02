#include "File.h"

#include "Exception.h"

#include <fstream>

namespace sxi::file
{
std::vector<char> readFileAsBytes(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw InvalidArgumentException("Failed to open file");

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
} // namespace sxi::file
