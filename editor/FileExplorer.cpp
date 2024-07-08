#include "FileExplorer.hpp"

jed::FileExplorer::FileExplorer()
{
    set_data_source(&m_data);
    update_text_data();
}

void jed::FileExplorer::handle_enter()
{
    m_viewer.set_directory(m_curr_index);
    update_text_data();
}

void jed::FileExplorer::handle_backspace()
{
    m_viewer.set_parent_directory();
    update_text_data();
}

void jed::FileExplorer::handle_arrow_up()
{
    m_curr_index -= 1;

    if (m_curr_index < 0) {
        m_curr_index = 0;
    }
}

void jed::FileExplorer::handle_arrow_down()
{
    m_curr_index += 1;

    if (m_curr_index >= m_viewer.size()) {
        m_curr_index = m_viewer.size() - 1;
    }
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

        data.append(std::get<std::string>(dirs));
        // data.append("\n");
        // data.replace(0, 14, "~/", 2);
    }

    m_data.append_string(data);
}