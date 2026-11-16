#include "LabrLut.h"
#include "Units.h"
#include <fstream>
#include <iostream>
#include <sstream>

void LabrLut::ReadLut(const std::string& file_name) {
    unsigned int  mapNumber;
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Labr Lut not present\n");
    unsigned int board, chan, nCal;
    double       thrLow, thrHigh;
    double       ph, th, tO, coeff;
    std::string  chanName;
    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;

        if(line.at(line.find_first_not_of(' ')) == '#') continue;

        std::stringstream strline(line);
        strline >> board >> chan >> mapNumber >> chanName >> thrLow >> thrHigh
            >> th >> ph >> tO >> nCal;
        // theta[board][chan]  = th*UNITS::deg;
        // phi[board][chan]    = (180.-ph)*UNITS::deg;
        prop[board][chan].pos.SetMagThetaPhi(409 * UNITS::mm, th * UNITS::deg,
                                             ph * UNITS::deg);
        prop[board][chan].posOrig    = prop[board][chan].pos;
        prop[board][chan].timeOffset = tO * 10 * UNITS::ns;
        prop[board][chan].thr        = std::make_pair(thrLow, thrHigh);
        prop[board][chan].name       = chanName;

        // Energy

        for(unsigned int i{0}; i < nCal; ++i) {
            if(!strline.good())
                throw std::runtime_error(
                    "Calibration coefficients are missing\n");
            strline >> coeff;
            (prop[board][chan].cal).push_back(coeff);
        }

        // Qshort
        if(strline.good()) {
            strline >> nCal;
            for(unsigned int i{0}; i < nCal; ++i) {
                strline >> coeff;
                (prop[board][chan].calQshort).push_back(coeff);
            }
        }
    }

    // for(const auto& b: prop){
    //	for(const auto& c: b.second){
    //	std::cout << "brd " << b.first << " chan " << c.first << " ncal " <<
    // c.second.cal.size() << " "; 	for (const auto& it2: c.second.cal){
    // std::cout
    //<< it2 << " ";
    //	}
    //	std::cout << std::endl;
    //	}
    // }
}

void LabrLut::PrintLut(std::ostream& outstream) {
    for(const auto& brd : prop) {
        for(const auto& chan : brd.second) {
            const auto& p{chan.second};
            outstream << brd.first << "	" << chan.first << "	"
                      << "MAPNR"
                      << "	" << p.name << "	" << p.thr.first
                      << "	" << p.thr.second << "	" << p.pos.Theta()
                      << "	" << p.pos.Phi() << "	"
                      << p.timeOffset / (10 * UNITS::ns) << "	"
                      << p.cal.size();
            for(const auto& val : p.cal) { outstream << "	" << val; }
            for(const auto& val : p.calQshort) { outstream << "	" << val; }
            outstream << "\n";
        }
    }
}
