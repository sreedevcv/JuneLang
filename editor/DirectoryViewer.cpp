#include "DirectoryViewer.hpp"

#include <iostream>

jed::DirectoryViewer::DirectoryViewer()
    : m_starting_dir(std::filesystem::current_path())
    , m_curr_dir(std::filesystem::current_path())
{
    std::cout << "dir: " << m_curr_dir.path().string() << "\n";
    update_dirents();
}

void jed::DirectoryViewer::set_directory(int index)
{
    m_curr_dir = std::filesystem::directory_entry(std::get<std::string>(m_dirents[index]));
    update_dirents();
}

void jed::DirectoryViewer::set_parent_directory()
{
    m_curr_dir = std::filesystem::directory_entry(m_curr_dir.path().parent_path());
    update_dirents();
}

const std::vector<jed::DirectoryViewer::entry_t>& jed::DirectoryViewer::get_dirents()
{
    return m_dirents;
}

void jed::DirectoryViewer::update_dirents()
{
    m_dirents.clear();
    for (const auto dir : std::filesystem::directory_iterator(m_curr_dir)) {
        std::cout << "dir: " << dir.path().string() << " " << dir.is_regular_file() << " " << dir.is_directory() << "\n";
        entry_t entry = { dir.path().string(), get_type(dir) };
        m_dirents.push_back(entry);
    }
}

int32_t jed::DirectoryViewer::size()
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
