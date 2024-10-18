#include "FontLoader.hpp"

#include "Context.hpp"

jed::FontLoader jed::FontLoader::font_loader = jed::FontLoader();

std::map<const char*, jed::Font*> jed::FontLoader::m_font_map = {
    { RES_PATH "/fonts/CascadiaMono.ttf", nullptr }
};

jed::FontLoader* jed::FontLoader::get()
{
    return &font_loader;
}

jed::Font*& jed::FontLoader::font(const char* name)
{
    return m_font_map[name];
}

void jed::FontLoader::load_fonts()
{
    for (auto [a, b] : m_font_map) {
        m_font_map[a] = new Font(a, Context::get().font_size);
        m_font_map[a]->load();
    }
}
