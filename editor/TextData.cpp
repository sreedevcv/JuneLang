#include "TextData.hpp"

void jed::TextData::add_text_to_line(const char* text, Cursor cursor)
{
    _ASSERT(cursor.line >= 0);

    // Add new lines
    if (cursor.line < m_data.size()) {
        for (int i = 0; i < m_data.size() - cursor.line; i++) {
            m_data.push_back("");
        }
    }

    std::string& line = m_data[cursor.line];

    if (cursor.loc == 0) {
        prepend_text(text, cursor.line);
    } else if (cursor.loc == line.size()) {
        append_text(text, cursor.line);
    } else {
        std::string lhs = line.substr(0, cursor.loc);
        std::string rhs = line.substr(cursor.loc, line.size());
        m_data[cursor.line] = lhs + text + rhs;
    }
}

void jed::TextData::append_text(const char* text, int line)
{
    m_data[line] += text;
}

void jed::TextData::prepend_text(const char* text, int line)
{
    m_data[line] = text + m_data[line];
}
