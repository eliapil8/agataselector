#include "SpiderLut.h"
#include "Units.h"
#include <fstream>
#include <iostream>
#include <sstream>

void SpiderLut::ReadLut(const std::string& file_name) {
    unsigned int  mapNumber;
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Spider Lut not present\n");
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
        prop[board][chan].pos.SetMagThetaPhi(90 * UNITS::mm, th * UNITS::deg,
                                             ph * UNITS::deg);
        prop[board][chan].posOrig    = prop[board][chan].pos;
        prop[board][chan].timeOffset = tO * 10 * UNITS::ns;
        prop[board][chan].thr        = std::make_pair(thrLow, thrHigh);
        prop[board][chan].name       = chanName;

        for(unsigned int i{0}; i < nCal; ++i) {
            strline >> coeff;
            (prop[board][chan].cal).push_back(coeff);
        }
    }
}

void SpiderLut::PrintLut(std::ostream& outstream) {
    assert(!fLut_file_fullname.empty());
    std::ifstream old_lut_file(fLut_file_fullname.c_str());

    if(!old_lut_file.is_open())
        throw std::runtime_error("Spider Lut " + fLut_file_fullname
                                 + " not present\n");
    unsigned int board, chan;
    std::string  chanName;
    std::string  line;

    while(std::getline(old_lut_file, line)) {
        if(line.empty()
           or line.find_first_not_of(" \t") == std::string::npos) {
            outstream << line << '\n';
            continue;
        }
        if(line.at(line.find_first_not_of(' ')) == '#') {
            outstream << line << '\n';
            continue;
        }
        std::vector<std::string> spaces;
        std::vector<std::string> words;

        Lut::SplitStringIntoWordsAndSpaces(line, words, spaces);

        std::stringstream strline(line);

        strline >> board >> chan;
        auto         params = prop[board][chan];

        // print until theta
        unsigned int iter   = 0;

        // sanity check
        assert(spaces.size() >= 10 + params.cal.size()
               && "not enough spaces in the line to print the lut");

        outstream << spaces[iter] << board;
        ++iter;
        outstream << spaces[iter] << chan;
        ++iter;
        outstream << spaces[iter] << params.map;
        ++iter;
        outstream << spaces[iter] << params.name;
        ++iter;
        outstream << spaces[iter] << params.thr.first;
        ++iter;
        outstream << spaces[iter] << params.thr.second;
        ++iter;
        outstream << spaces[iter] << params.pos.Theta() / UNITS::deg;
        ++iter;
        outstream << spaces[iter] << params.pos.Phi() / UNITS::deg;
        ++iter;
        outstream << spaces[iter] << params.timeOffset / (10 * UNITS::ns);
        ++iter;
        outstream << spaces[iter] << params.cal.size();
        ++iter;
        for(auto& cal : params.cal) {
            outstream << spaces[iter] << cal;
            ++iter;
        }
        // now we should have "iter" number of spaces and "iter" number of
        // words printed, but there might be a comment at the end of the line
        for(; iter < spaces.size(); iter++) {
            outstream << spaces[iter];
            if(iter < words.size()) { outstream << words[iter]; }
        }
        outstream << '\n';
    }
}
