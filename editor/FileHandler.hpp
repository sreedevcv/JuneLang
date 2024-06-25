#pragma once

#include <fstream>
#include <string>

#include "TextData.hpp"

namespace jed {

class FileHandler {
public:
    FileHandler() = default;
    ~FileHandler() = default;


    bool open_and_read(std::string& file_name);
    TextData& get_text_data();

private:
    std::fstream m_file;
    std::string m_name;
    // std::string m_contents;
    TextData m_contents;
};

} // namespace jed
