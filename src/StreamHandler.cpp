#include "StreamHandler.hpp"

jl::StreamHandler::StreamHandler()
    : currentStreamType(StreamType::Cout)
    , fileStream(nullptr)
{
}

jl::StreamHandler::~StreamHandler()
{
    if (fileStream) {
        delete fileStream;
    }
}

void jl::StreamHandler::setOutputToCout()
{
    currentStreamType = StreamType::Cout;
}

void jl::StreamHandler::setOutputToCerr()
{
    currentStreamType = StreamType::Cerr;
}

void jl::StreamHandler::setOutputToStr()
{
    currentStreamType = StreamType::Str;
}

void jl::StreamHandler::setOutputToFile(const std::string& filename)
{
    if (fileStream) {
        delete fileStream;
    }
    fileStream = new std::ofstream(filename, std::ios::out | std::ios::trunc);
    if (!fileStream->is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    currentStreamType = StreamType::File;
}

jl::StreamHandler& jl::StreamHandler::operator<<(std::ostream& (*manip)(std::ostream&))
{
    std::ostream& stream = getCurrentStream();
    stream << manip;
    return *this;
}

std::stringstream& jl::StreamHandler::get_string_stream()
{
    if (currentStreamType != StreamType::Str) {
        std::cout << "FATAL ERROR : Cannot extract string stream from a non string stream StreamHandler" << std::endl;
        std::exit(1);
    }
    return sstream;
}

std::ostream& jl::StreamHandler::getCurrentStream()
{
    switch (currentStreamType) {
    case StreamType::Cout:
        return std::cout;
    case StreamType::Cerr:
        return std::cerr;
    case StreamType::Str:
        return sstream;
    case StreamType::File:
        if (fileStream && fileStream->is_open()) {
            return *fileStream;
        } else {
            throw std::runtime_error("File stream is not open.");
        }
    }
    // Default to cout in case of unexpected errors
    return std::cout;
}
