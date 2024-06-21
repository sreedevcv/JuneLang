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

    void add_text_to_line(char text, Cursor cursor);

private:
    struct str {
        char* data;
        int size;
        int capacity;
    };

    void append_text(char text, int line);
    void prepend_text(char text, int line);
    std::vector<str> m_data;

    friend class TextRender;
    friend class Editor;
};

} // namespace jed
