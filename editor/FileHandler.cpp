#include "FileHandler.hpp"

#include <iostream>
#include <cstring>

#include "Context.hpp"

bool jed::FileHandler::open_and_read(std::string& file_name)
{
    m_file = std::fstream(file_name, std::ios::in);
    m_contents.m_data.clear();

    if (!m_file.good()) {
        return false;
    }

    std::string line;
    bool has_contents = false;

    while (m_file.good()) {
        if (!has_contents) {
            m_contents.clear();
            has_contents = true;
        }

        std::getline(m_file, line);

        int capacity_factor = (line.size() / Context::get().data_grow_size) + 1;
        m_contents.m_data.push_back(TextData::str {
            .data = (char*) malloc(sizeof(char) * capacity_factor * Context::get().data_grow_size),
            .size = static_cast<int>(line.size()),
            .capacity = capacity_factor * Context::get().data_grow_size
        });
        m_contents.m_line_count += 1;

        std::memmove(m_contents.m_data.back().data, line.c_str(), line.size());
    }

    return true;
}

jed::TextData& jed::FileHandler::get_text_data()
{
    return m_contents;
}
