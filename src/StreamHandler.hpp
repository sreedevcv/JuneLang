#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

namespace jl {

class StreamHandler {
public:
    enum class StreamType {
        Cout,
        Cerr,
        File,
        Str,
    };

    StreamHandler();
    ~StreamHandler();

    void setOutputToCout();
    void setOutputToCerr();
    void setOutputToStr();
    void setOutputToFile(const std::string& filename);

    template <typename T>
    StreamHandler& operator<<(const T& value)
    {
        std::ostream& stream = getCurrentStream();
        stream << value;
        return *this;
    }

    // Overload for manipulators like std::endl
    StreamHandler& operator<<(std::ostream& (*manip)(std::ostream&));

    std::stringstream& get_string_stream();

private:
    StreamType currentStreamType;
    std::ofstream* fileStream;
    std::stringstream sstream;

    std::ostream& getCurrentStream();
};

}
