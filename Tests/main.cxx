#include <iostream>
#include <sstream>
#include <vector>

#include "Fitter.h"
#include "Launcher.h"

#include <TApplication.h>

struct Properties {
    std::string spectra;
    double      transition_energy{};
    double      max_centroid_dicr{};
    double      max_sigma{};
    double      integral{};
    double      max_integral_discr{};
};

std::vector<Properties> getProperties(const std::string&);

int                     main(int argc, char* argv[]) {
    std::string path = argv[1];
    bool        enableCv{false};
    if(argc > 2 && std::string(argv[2]) == "--canvas") enableCv = true;
    RooMsgService::instance().setSilentMode(true);
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

    std::string command = "../RunSelector --verb 1 --conf ./Tests/";
    command += path;
    command += "/selector.conf ";
    command += "--iofile ./Tests/";
    command += path;
    command += "/red_Tree_0000.root test.root";

    std::cout << command << std::endl;

    // Runs the selector producing test.root
    remove("./test.root");
    auto returnVal = system(command.c_str());
    if(returnVal != 0) {
            std::cout << "Selector failed to run\n";
            return returnVal;
    }

    // Fit

    auto  prop = getProperties("./Tests/" + path + "/transitions.txt");
    TFile f("test.root", "r");
    if(!f.IsOpen()) throw std::runtime_error("Unable to find file\n");
    int          returnValue{-1};
    TApplication root_app("Tester", &argc, argv);
    for(const auto& it : prop) {
        if(returnValue == -1) returnValue = 0;
        std::vector<std::pair<double, double>> energies;
        energies.emplace_back(it.transition_energy, 100);

        Fitter fitter(energies, Fitter::Tail::LEFT);

        if(enableCv) fitter.EnableCanvas();

        TH1D* h = dynamic_cast<TH1D*>(f.Get(it.spectra.c_str()));
        fitter.SetSpec(*h);

        auto res = fitter.Fit();

        if(abs(res.at(0).mean_gauss.first - it.transition_energy)
           > it.max_centroid_dicr) {
            std::cout << "Energy  " << it.transition_energy
                      << " Mean discr of "
                      << res.at(0).mean_gauss.first - it.transition_energy
                      << "(" << it.max_centroid_dicr << ")" << std::endl;
            returnValue++;
        }
        if(sqrt(pow(res.at(0).sigma_gauss.first, 2)
                                    + pow(res.at(0).tail_left.first, 2))
           > it.max_sigma) {
            std::cout << "Sigma of "
                      << sqrt(pow(res.at(0).sigma_gauss.first, 2)
                                                  + pow(res.at(0).tail_left.first, 2))
                      << "(" << it.max_sigma << ")" << std::endl;
            returnValue++;
        }
        if(abs(res.at(0).counts.first - it.integral) > it.max_integral_discr) {
            std::cout << "Int discr of "
                      << res.at(0).counts.first - it.integral << "("
                      << it.max_integral_discr << ")" << std::endl;
            returnValue++;
        }
    }

    std::cout << "Return value " << returnValue << std::endl;
    return returnValue;
    // return 0;
}

std::vector<Properties> getProperties(const std::string& fileName) {
    std::ifstream           file(fileName);
    std::string             line;
    std::vector<Properties> properties;

    while(std::getline(file, line)) {
        if(line[0] == '#') continue;

        Properties         tmp;
        std::istringstream sstr(line);
        sstr >> tmp.spectra >> tmp.transition_energy >> tmp.max_centroid_dicr
            >> tmp.max_sigma >> tmp.integral >> tmp.max_integral_discr;
        properties.emplace_back(tmp);
    }
    return properties;
}
