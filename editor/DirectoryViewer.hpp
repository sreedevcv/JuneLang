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

    bool empty() const;
    std::string path() const;
    void set_directory(int index);
    void set_parent_directory();
    const std::vector<entry_t>& get_dirents() const;
    int32_t size() const;

private:
    std::filesystem::directory_entry m_starting_dir;
    std::filesystem::directory_entry m_curr_dir;
    std::vector<entry_t> m_dirents;
    bool m_dir_empty = false;

    Type get_type(const std::filesystem::directory_entry& direnty) const;
    void update_dirents();
};

} // namespace jed
