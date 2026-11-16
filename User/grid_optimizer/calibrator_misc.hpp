#ifndef CALIBRATOR_MISC_HPP
#define CALIBRATOR_MISC_HPP

#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>


namespace fs = std::filesystem;

namespace gridopti
{
void createDirectories(const std::string &path)
{
    fs::path originalPath = fs::current_path(); 
    fs::path currentPath;
    try
    {
        for (const auto &part : fs::path(path))
        {
            currentPath = fs::current_path() / part;

            if (!fs::exists(currentPath) || !fs::is_directory(currentPath))
            {
                fs::create_directory(currentPath);
            }
            fs::current_path(currentPath);
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    fs::current_path(originalPath);
}

std::string searchDirectory(const fs::path &directory, const std::string &str1, const std::string &str2)
{
    for (const auto &entry : fs::directory_iterator(directory))
    {
        std::string path = entry.path();
        if (fs::is_regular_file(path) && path.find(str1) != std::string::npos && path.find(str2) != std::string::npos) { return path; }
    }
    return "";
}

void RemoveDoubleSlashes(std::string &str)
{
    auto found_slashes = str.find("//");
    while (found_slashes != std::string::npos)
    {
        str.replace(found_slashes, 2, "/");
        found_slashes = str.find("//", found_slashes + 1);
    }
}


template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return std::move(out).str();
}

} // namespace gridopti

#endif