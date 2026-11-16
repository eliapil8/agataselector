#pragma once

#include <TObject.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

class Header : public TObject {
  public:
    // Header() = delete;
    Header()           = default;
    ~Header() override = default;

    /// @brief Create new header (this should be a constructor but ROOT has
    /// some limitation on what TObject classes can do in the constructors)
    /// @param conf_file_name name of the configuration file that is to be
    /// stored
    /// @param comments some additional comments
    void CreateHeader(const std::string& conf_file_name,
                      const std::string& comments = "");
    void AddLut(const std::string& file_name, const std::string& detector);

    void GetCreationDate();                // *MENU*
    void GetComment();                     // *MENU*
    void GetGitHash();                     // *MENU*
    void GetFileName();                    // *MENU*
    void GetFileContent();                 // *MENU*
    void Dump() const override;            // *MENU*
    void Print(Option_t*) const override { // *MENU*
        Dump();
    };
    void Browse(TBrowser*) override { Dump(); };

  private:
    std::string                                  creationTime;
    std::string                                  comments;
    std::string                                  gitDetails;
    std::pair<std::string, std::string>          confFile;
    std::unordered_map<std::string, std::string> lutFile;
    ClassDefOverride(Header, 2);
};

void Header::Dump() const {
    std::cout << creationTime << "\n"
              << comments << "\n"
              << gitDetails << "\n"
              << "----------------------------------------\n"
              << confFile.first << "\n"
              << confFile.second << "\n";
    for(const auto& it : lutFile) {
        std::cout << "----------------------------------------\n"
                  << it.first << "\n"
                  << it.second << "\n";
    }
}

void Header::CreateHeader(const std::string& conf_file_name,
                          const std::string& comm) {
    // time now
    time_t     rawtime;
    struct tm* timeinfo;
    char       buf[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "Run at %d/%m/%Y %H:%M:%S", timeinfo);
    creationTime   = std::string(buf);

    // comments
    this->comments = !comm.empty() ? comm : "No comments given";

    // read conf file
    confFile.first = conf_file_name;
    std::ifstream infile(confFile.first);
    if(!infile.good())
        throw std::runtime_error("Conf file in header not present!?\n");
    std::stringstream buffer;
    buffer << infile.rdbuf();
    confFile.second = buffer.str();

    // get git hash
#ifdef GIT_COMMIT_HASH
    gitDetails = std::string("Git branch: ") + GIT_BRANCH
               + std::string(", commit: ") + GIT_COMMIT_HASH;
#else
    gitDetails = "Could not capture git hash";
#endif
}

void Header::AddLut(const std::string& file_name,
                    const std::string& detector) {
    // read lut file
    std::ifstream infile(file_name);
    if(!infile.good())
        throw std::runtime_error("Lut file " + file_name
                                 + " in header not present!?\n");
    std::stringstream buffer;
    buffer << infile.rdbuf();
    lutFile[detector] = buffer.str();
}

void Header::GetComment() { std::cout << comments << std::endl; }

void Header::GetCreationDate() { std::cout << creationTime << std::endl; }

void Header::GetGitHash() { std::cout << gitDetails << std::endl; }

void Header::GetFileName() { std::cout << confFile.first << std::endl; }

void Header::GetFileContent() { std::cout << confFile.second << std::endl; }
