#include "GammadetectorSpiderLut.h"
#include "Units.h"
#include <fstream>
#include <iostream>
#include <sstream>

void GammadetectorSpiderLut::ReadLut(const std::string& file_name) {
    // unsigned int mapNumber;
    std::string   line;
    std::ifstream file(file_name);

    if(!file.is_open())
        throw std::runtime_error("Not possible to open GammadetectorSpiderLut "
                                 + file_name + "\n");

    ExGate       tmp;

    unsigned int board, chan;
    double       thrLow, thrHigh;

    while(std::getline(file, line)) {
        // std::cout << line <<  std::endl;
        if(line.empty()
           or line.find_first_not_of(" \t") == std::string::npos) {
            // std::cout << "line.empty() or line.find_first_not_of('  ') : "
            // << line << std::endl;
            continue;
        }

        if(line.at(line.find_first_not_of(' ')) == '#') {
            //      std::cout << "Comment:   " << line << std::endl;
            continue;
        }

        std::stringstream strline(line);

        strline >> board >> chan >> thrLow >> thrHigh;
        tmp[std::make_pair(board, chan)].first  = thrLow;
        tmp[std::make_pair(board, chan)].second = thrHigh;
    }

    exGate.push_back(tmp);
}

void GammadetectorSpiderLut::PrintLut(std::ostream& outstream) {
   outstream << "GammadetectorSpiderLut::PrintLut not implemented\n";
    throw std::runtime_error("Not implemented\n");
}
