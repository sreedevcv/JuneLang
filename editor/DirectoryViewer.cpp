#include "DirectoryViewer.hpp"

#include <iostream>

jed::DirectoryViewer::DirectoryViewer()
    : m_starting_dir(std::filesystem::current_path())
    , m_curr_dir(std::filesystem::current_path())
{
    update_dirents();
}

bool jed::DirectoryViewer::empty() const
{
    return m_dir_empty;
}

std::string jed::DirectoryViewer::path() const
{
    return m_curr_dir.path().string();
}

void jed::DirectoryViewer::set_directory(int index)
{
    m_curr_dir = std::filesystem::directory_entry(std::get<std::string>(m_dirents[index]));
    update_dirents();
}

void jed::DirectoryViewer::set_parent_directory()
{
    auto parent_path = m_curr_dir.path().parent_path();
    m_curr_dir = std::filesystem::directory_entry(parent_path);
    update_dirents();
}

const std::vector<jed::DirectoryViewer::entry_t>& jed::DirectoryViewer::get_dirents() const
{
    return m_dirents;
}

void jed::DirectoryViewer::update_dirents()
{
    m_dirents.clear();
    auto iterator = std::filesystem::directory_iterator(m_curr_dir.path());
    for (const auto dir : iterator) {
        entry_t entry = { dir.path().string(), get_type(dir) };
        m_dirents.push_back(entry);
    }

    m_dir_empty = m_dirents.empty();
}

int32_t jed::DirectoryViewer::size() const
{
    return m_dirents.size();
}

jed::DirectoryViewer::Type jed::DirectoryViewer::get_type(const std::filesystem::directory_entry& direnty) const
{
    if (direnty.is_directory()) {
        return DIR;
    }
    if (direnty.is_regular_file()) {
        return FILE;
    }
    return NONE;
}
