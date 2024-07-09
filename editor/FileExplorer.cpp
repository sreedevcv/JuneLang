#include "FileExplorer.hpp"

jed::FileExplorer::FileExplorer()
{
    set_data_source(&m_data);
    update_text_data();

    if (!m_viewer.empty()) {
        m_curr_index = 0;
        m_data.line(0)[0] = '*';
    }
}

void jed::FileExplorer::handle_enter()
{
    if (m_viewer.empty()) {
        return;
    }

    DirectoryViewer::Type type = std::get<DirectoryViewer::Type>(m_viewer.get_dirents()[m_curr_index]);
    if (type == DirectoryViewer::DIR) {
        m_viewer.set_directory(m_curr_index);
        update_text_data();
        m_curr_index = 0;
        m_data.line(0)[0] = '*';
    } else {
        m_just_selected_file = true;
    }

}

void jed::FileExplorer::handle_backspace()
{
    m_viewer.set_parent_directory();
    update_text_data();

    if (m_viewer.empty()) {
        return;
    }

    m_curr_index = 0;
    m_data.line(0)[0] = '*';
}

bool jed::FileExplorer::has_selected_file()
{
    bool ret_value = m_just_selected_file;

    if (m_just_selected_file) {
        m_just_selected_file = false;
    }

    return ret_value;
}

std::string jed::FileExplorer::get_path()
{
    return std::get<std::string>(m_viewer.get_dirents()[m_curr_index]);
}

void jed::FileExplorer::handle_arrow_up()
{
    if (m_viewer.empty()) {
        return;
    }

    m_data.line(m_curr_index)[0] = ' ';
    m_curr_index -= 1;

    if (m_curr_index < 0) {
        m_curr_index = 0;
    }

    m_data.line(m_curr_index)[0] = '*';
}

void jed::FileExplorer::handle_arrow_down()
{    
    if (m_viewer.empty()) {
        return;
    }

    m_data.line(m_curr_index)[0] = ' ';
    m_curr_index += 1;

    if (m_curr_index >= m_viewer.size()) {
        m_curr_index = m_viewer.size() - 1;
    }

    m_data.line(m_curr_index)[0] = '*';
}

void jed::FileExplorer::update_text_data()
{
    m_data.clear();
    std::string data;

    for (const auto& dirs : m_viewer.get_dirents()) {
        switch (std::get<DirectoryViewer::Type>(dirs)) {
        case DirectoryViewer::Type::FILE:
            data.append(" [F] ");
            break;
        case DirectoryViewer::Type::DIR:
            data.append(" [D] ");
            break;
        case DirectoryViewer::Type::NONE:
            data.append(" [?] ");
            break;
        default:
            std::cout << "Unimplemented DirectoryViewer::Type found in jed::FileExplorer::update_text_data()" << std::endl;
            std::exit(1);
            break;
        }

        std::string path = std::get<std::string>(dirs);
        data.append(file_name(path));
        data.append("\n");
    }

    m_data.append_string(data);
}

const char* jed::FileExplorer::file_name(std::string& full_path)
{
    int slash_index = full_path.size() - 1;
    while (slash_index >= 0) {
        if (full_path[slash_index] == '/') {
            break;
        }

        slash_index -= 1;
    }

    return full_path.c_str() + slash_index + 1;
}