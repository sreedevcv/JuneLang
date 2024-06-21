#include "TextData.hpp"

#include <iostream>

void jed::TextData::add_text_to_line(char text, Cursor cursor)
{
    _ASSERT(cursor.line >= 0);
    int current_size = m_data.size();

    // Add new lines
    if (cursor.line <= current_size) {
        for (int i = 0; i < current_size - cursor.line + 1; i++) {
            // m_data.push_back("");
            // m_data.back().reserve(20);
        }
    }

    // std::string& line = m_data[cursor.line];

    // if (cursor.loc == 0) {
    //     prepend_text(text, cursor.line);
    // } else if (cursor.loc == line.size()) {
        // append_text(text, cursor.line);
    // } else {
    //     std::string lhs = line.substr(0, cursor.loc);
    //     std::string rhs = line.substr(cursor.loc, line.size());
    //     m_data[cursor.line] = lhs + text + rhs;
    // }

    std::cout << "Text appeded: " << text << std::endl;
}

void jed::TextData::append_text(char text, int line)
{
    // m_data[line] += text;
}

void jed::TextData::prepend_text(char text, int line)
{
    // m_data[line] = text + m_data[line];
}
