#pragma once

#include <string>
#include <vector>

#include <TFile.h>

class App {
  public:
    App();
    std::vector<std::string> finalFiles{};

  private:
    static void          sigintFunction(int);
    void                 Spy(TFile& file, std::atomic<bool>& terminate);

    /** Add OSCAR E-dE overlay canvases to completed output files serially. */
    void WriteOscarEdECanvases(const std::vector<std::string> &files) const;
};
