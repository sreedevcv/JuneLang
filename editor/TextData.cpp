#include "TextData.hpp"

#include <iostream>

void jed::TextData::add_text_to_line(char text, Cursor cursor)
{
    _ASSERT(cursor.line >= 0);
    int current_size = m_data.size();

    // Add new lines
    if (cursor.line >= current_size) {
        for (int i = 0; i < cursor.line - current_size + 1; i++) {
            m_data.push_back(str {
                .data = (char*) malloc(sizeof(char) * 100),
                .size = 0,
                .capacity = 100
            });
        }
    }

    // std::string& line = m_data[cursor.line];

    // if (cursor.loc == 0) {
    //     prepend_text(text, cursor.line);
    // } else if (cursor.loc == line.size()) {
        append_text(text, cursor.line);
    // } else {
    //     std::string lhs = line.substr(0, cursor.loc);
    //     std::string rhs = line.substr(cursor.loc, line.size());
    //     m_data[cursor.line] = lhs + text + rhs;
    // }

    std::cout << "Text appeded: " << text << " " << m_data[cursor.line].size << " " << m_data[cursor.line].capacity << " " << m_data.size() << std::endl;
}

void jed::TextData::append_text(char text, int line)
{
    str& s = m_data[line];
    if (s.size < s.capacity) {
        s.data[s.size++] = text;
    }
}

void jed::TextData::prepend_text(char text, int line)
{
    // m_data[line] = text + m_data[line];
}
