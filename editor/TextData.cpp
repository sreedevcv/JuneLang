#include "TextData.hpp"

#include <iostream>

void jed::TextData::add_text_to_line(char text, Cursor cursor)
{
    int current_size = m_data.size();

    // Add new lines
    if (cursor.line >= current_size) {
        for (int i = 0; i < cursor.line - current_size + 1; i++) {
            m_data.push_back(str {
                .data = (char*)malloc(sizeof(char) * m_data_grow_size),
                .size = 0,
                .capacity = m_data_grow_size });
        }
    }

    if (cursor.loc >= m_data[cursor.line].size) {
        append_text(text, cursor.line);
    } else {
        str& s = m_data[cursor.line];
        if (s.size < s.capacity) {
            shift_one_back(cursor.loc, cursor.line);
            s.data[cursor.loc] = text;
            s.size++;
        }
    }

    // std::string& line = m_data[cursor.line];

    // if (cursor.loc == 0) {
    // prepend_text(text, cursor.line);
    // } else if (cursor.loc == line.size()) {
    // append_text(text, cursor.line);
    // } else {
    //     std::string lhs = line.substr(0, cursor.loc);
    //     std::string rhs = line.substr(cursor.loc, line.size());
    //     m_data[cursor.line] = lhs + text + rhs;
    // }

    std::cout << "Text appeded: " << text << "|size: " << m_data[cursor.line].size << "|cap: " << m_data[cursor.line].capacity << "|cur: " << cursor.loc << std::endl;
    // std::cout << m_data[cursor.line].data << std::endl;
}

void jed::TextData::append_text(char text, int line)
{
    str& s = m_data[line];
    if (s.size < s.capacity) {
        s.data[s.size++] = text;
    } else {
        s.data = (char*)realloc(s.data, sizeof(char) * (s.capacity + m_data_grow_size));
        s.data[s.size++] = text;
    }
}

void jed::TextData::prepend_text(char text, int line)
{
    str& s = m_data[line];

    if (s.size < s.capacity) {
        shift_one_back(0, line);
        s.data[0] = text;
        s.size++;
    }
}

void jed::TextData::shift_one_back(int start, int line)
{
    str& s = m_data[line];
    for (int i = s.size - 1; i >= start; i--) {
        s.data[i + 1] = s.data[i];
    }
}
