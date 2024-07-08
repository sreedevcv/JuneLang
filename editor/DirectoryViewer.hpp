#pragma once

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

namespace jed {

class DirectoryViewer {
public:
    enum Type {
        DIR,
        FILE,
        NONE,
    };

    using entry_t = std::tuple<std::string, Type>;

    DirectoryViewer();
    ~DirectoryViewer() = default;

    void set_directory(int index);
    void set_parent_directory();
    const std::vector<entry_t>& get_dirents();
    int32_t size();

private:
    std::filesystem::directory_entry m_starting_dir;
    std::filesystem::directory_entry m_curr_dir;
    std::vector<entry_t> m_dirents;

    Type get_type(const std::filesystem::directory_entry& direnty) const;
    void update_dirents();
};

} // namespace jed
