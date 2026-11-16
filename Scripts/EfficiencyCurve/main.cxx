#include <iostream>

#include <TRint.h>

#include "SpectrumAnalyzer.h"

int main(int argc, char* argv[]) {
    auto interface = new TRint("App", nullptr, nullptr);
    interface->SetPrompt("\u001b[1;32mcmd [%d]\u001b[1;0m ");

    // make sure that the Gpad and GUI libs are loaded
    TRint::NeedGraphicsLibs();
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();

    std::cout << "Starting analysis now \n";
    if (argc < 2)    throw std::runtime_error("set inputs correctly\n");
    bool debug_canvas = false;
    bool use_main_transitions = false;
    bool read_from_file = false;
    bool ba = false;
    bool eu = false;
    std::string hist_name = "energy_sum";
    std::vector<std::string> files;
    for (int ii=1; ii<argc;++ii){
        std::string val(argv[ii]);
        if (val.find("-h") != std::string::npos) {
            std::cout << "--debug-canvas\t\t\t\tshows canvas with fits\n";
            std::cout << "--read-from-file\t\t\tRead fit parameters from file\n";
            std::cout << "--use-main-transitions\t\t\tuses only transitions with intensity > 0.5\n";
            std::cout << "--eu\t\t\t\t\tAssume 152Eu spectrum\n";
            std::cout << "--ba\t\t\t\t\tAssume 133Ba spectrum\n";
            std::cout << "--spec-name=hist_name\t\t\tspecify the histogram name (default energy_sum)\n";
            return 0;
        }
        if (val.find("--debug-canvas") != std::string::npos) {
            debug_canvas = true;
            continue;
        }
        if (val.find("--use-main-transitions") != std::string::npos) {
            use_main_transitions = true;
            continue;
        }
        if (val.find("--read-from-file") != std::string::npos) {
            read_from_file = true;
            continue;
        }
        if (val.find("--ba") != std::string::npos) {
            ba = true;
            continue;
        }
        if (val.find("--eu") != std::string::npos) {
            eu = true;
            continue;
        }
        if (val.find("--spec-name") != std::string::npos) {
            hist_name = val.substr(val.find("=") + 1);
            continue;
        }
        files.push_back(val);
    }

    if(!eu && !ba) throw std::runtime_error("Select either ba or eu\n");
    for (const auto& file:files){
        std::cout << "Running analysis, file : " << file << "\n";
    
        if(eu){
            SpectrumAnalyzer analyze(file, 
                hist_name,
                debug_canvas, 
                use_main_transitions, 
                read_from_file,
                "files/adoptedLevels152Sm_withalpha.csv",
                "files/intensity152Eu.txt"
            );
        }
        if(ba){
            SpectrumAnalyzer analyze(file,
                hist_name, 
                debug_canvas, 
                use_main_transitions, 
                read_from_file,
                "files/adoptedLevels133Cs.csv",
                "files/intensity133Ba.txt"
            );

        }
    }
    std::cout << "Finished the analysis\n";
    return 0;
}