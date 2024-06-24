#include "TextData.hpp"

#include <iostream>

void jed::TextData::add_text_to_line(char text, Cursor cursor)
{
    int current_size = m_data.size();

    // Add new lines
    if (cursor.line >= current_size) {
        for (int i = 0; i < cursor.line - current_size + 1; i++) {
            m_data.push_back(make_new_str());
        }
    }

    if (cursor.loc >= m_data[cursor.line].size) {
        append_text(text, cursor.line);
    } else {
        str& s = m_data[cursor.line];
        if (s.size >= s.capacity) {
            grow_line(cursor.line);
        }

        shift_one_back(cursor.loc, cursor.line);
        s.data[cursor.loc] = text;
        s.size++;
    }

    std::cout << "Text appeded: " << text << "|size: " << m_data[cursor.line].size << "|cap: " << m_data[cursor.line].capacity << "|cur: " << cursor.loc << std::endl;
    // std::cout << m_data[cursor.line].data << std::endl;
}

void jed::TextData::handle_enter(Cursor cursor)
{
    // Insert a new line
    insert_line(cursor.line);

    // Reserve the necessary memory
    int capacity_count = ((m_data[cursor.line].size - cursor.loc) / m_data_grow_size) + 1;
    int capacity = capacity_count * m_data_grow_size;
    int size = m_data[cursor.line].size - cursor.loc;

    str s = {
        .data = (char*)malloc(sizeof(char) * capacity),
        .size = size,
        .capacity = capacity
    };
    
    // Copy contents to new line
    for (int i = cursor.loc; i < m_data[cursor.line].size; i++) {
        s.data[i - cursor.loc] = m_data[cursor.line].data[i];
    }

    m_data[cursor.line + 1] = s;
    m_data[cursor.line].size = cursor.loc;
}

void jed::TextData::handle_backspace(Cursor& cursor)
{
    if (cursor.loc > 0) {
        for (int i = cursor.loc - 1; i < m_data[cursor.line].size - 1; i++) {
            m_data[cursor.line].data[i] = m_data[cursor.line].data[i + 1];
        }
        m_data[cursor.line].size--;
        cursor.loc -= 1;
    } else {
        if (cursor.line == 0) {
            return;
        }

        int old_size = m_data[cursor.line].size;

        // Copy all contents to the above line
        for (int i = 0; i < m_data[cursor.line].size; i++) {
            append_text(m_data[cursor.line].data[i], cursor.line - 1);
        }
        // Delete the current line
        m_data[cursor.line].size = 0;
        str deleted_line = m_data[cursor.line];

        for (int i = cursor.line; i < get_line_count() - 1; i++) {
            m_data[i] = m_data[i + 1];
        }
        m_data[get_line_count() - 1] = deleted_line;
        cursor.line -= 1;
        cursor.loc = m_data[cursor.line].size - old_size;
    }
}

int jed::TextData::get_line_size(int line)
{
    return m_data[line].size;
}

int jed::TextData::get_line_count()
{
    return m_data.size();
}

void jed::TextData::bound_cursor_loc(Cursor& cursor)
{
    if (cursor.loc > get_line_size(cursor.line)) {
        cursor.loc = get_line_size(cursor.line);
    }
}

void jed::TextData::append_text(char text, int line)
{
    str& s = m_data[line];
    if (s.size < s.capacity) {
        s.data[s.size++] = text;
    } else {
        grow_line(line);
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

void jed::TextData::grow_line(int line)
{
    str& s = m_data[line];
    s.capacity += m_data_grow_size;
    s.data = (char*)realloc(s.data, sizeof(char) * s.capacity);
}

void jed::TextData::insert_line(int pos)
{
    m_data.push_back(str {});
    for (int i = m_data.size() - 2; i >= pos; i--) {
        m_data[i + 1] = m_data[i];
    }
}

jed::TextData::str jed::TextData::make_new_str()
{
    return str {
        .data = (char*)malloc(sizeof(char) * m_data_grow_size),
        .size = 0,
        .capacity = m_data_grow_size
    };
}
