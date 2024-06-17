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
        File
    };

    StreamHandler();
    ~StreamHandler();

    void setOutputToCout();
    void setOutputToCerr();
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

private:
    StreamType currentStreamType;
    std::ofstream* fileStream;

    std::ostream& getCurrentStream();
};

}
