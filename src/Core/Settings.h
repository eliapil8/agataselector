#pragma once

#include <iostream>
#include <string>
#include <vector>

class Settings {
  public:
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    Settings(const Settings&)            = delete;
    Settings& operator=(const Settings&) = delete;

  private:
    Settings()  = default;
    ~Settings() = default;

  public:
    std::ostream&             operator<<(std::ostream& os) const;
    std::string               confFileName{"./selector.conf"};
    std::vector<std::string>  inFileName;
    std::vector<std::string>  outFileName;
    std::string               ioListFile{};
    std::string               cwd{};
    std::vector<unsigned int> runNr;
    std::string               reduction_condition{};
    std::string               set_gates{};
    long long                 nrEvts{-1};
    unsigned int              nrThr{6};
    bool                      userSelector{true};
    bool                      remove_partial{false};
    bool                      distribute{false};
    bool                      printHelp{false};
    bool                      optimize{false};
    bool                      only_enabled_histos{false};
    bool                      debug_canvas{false};
    bool                      update_runs{false};
    bool                      no_hadd{false};
    bool                      mem_check{false};
    bool                      update_prisma{false};
    bool                      sum_all{false};
    bool                      subtract_bkg{false};
    int                       verbosity_level{0};
    int                       window_size{-1};
    std::string               printFileName{};
    std::string               spy{""};
    std::vector<std::string>  options;

    // Methods
  private:
    void readListFile(const std::string&);

  public:
    void parseOptions(int argc, char* argv[]);
};
