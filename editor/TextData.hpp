#pragma once

#include <string>
#include <vector>

namespace jed {

struct Cursor {
    int line;
    int loc;
};

class TextData {
public:
    TextData() = default;
    ~TextData() = default;

    void add_text_to_line(const char* text, Cursor cursor);

private:
    void append_text(const char* text, int line);
    void prepend_text(const char* text, int line);
    std::vector<std::string> m_data;

    friend class TextRender;
    friend class Editor;
};

} // namespace jed
