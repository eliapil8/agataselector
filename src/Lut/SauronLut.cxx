#include "SauronLut.h"

#include <sstream>

/*Naming scheme for channels:
          +y (ceiling)
       ---------
      /  -----  \
     /  /     \  \
    |  | beam  |  |
 +x |  |  +z   |  |
     \  \     /  /
      \  -----  /
       ---------

  PSIDECH00 -> P side, Channel 0
  NSIDECH13 -> N side, Channel 13

  if P side:
      quadrant = channel / 16
      ring = channel % 16
*/

SauronLut::SauronLut() : settings(Settings::getInstance()) {}

void SauronLut::ReadLut(const std::string& file_name) {
    fLut_file_fullname = file_name;
    prop.clear();
    unsigned int  mapNumber;
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Sauron Lut not present\n");
    unsigned int board, chan, nCal;
    double       thrLow, thrHigh;
    double       rad, ph, t0, coeff;
    std::string  chanName;
    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;

        if(line.at(line.find_first_not_of(' ')) == '#') continue;

        std::stringstream strline(line);
        strline >> board >> chan >> mapNumber >> chanName >> thrLow >> thrHigh
            >> rad >> ph >> t0 >> nCal;

        switch(mapNumber / 100) {
        case 0: prop[board][chan].side = SauronSide::PSIDE; break;
        case 1: prop[board][chan].side = SauronSide::NSIDE; break;
        default:
            throw std::runtime_error("In Sauron Lut: map number first digit "
                                     "(side) is not correct\n");
            break;
        }

        prop[board][chan].segment    = mapNumber % 100;
        prop[board][chan].name       = chanName;
        prop[board][chan].thr        = std::make_pair(thrLow, thrHigh);
        prop[board][chan].radius     = rad * UNITS::mm;
        prop[board][chan].phi        = ph * UNITS::deg;
        prop[board][chan].timeOffset = t0 * 10 * UNITS::ns;
        prop[board][chan].map        = mapNumber;

        for(unsigned int i{0}; i < nCal; ++i) {
            strline >> coeff;
            prop[board][chan].cal.push_back(coeff);
        }
    }

    if(settings.verbosity_level > 2) {
        std::cout << "Printing SAURON lut" << std::endl;
        for(const auto& it : prop) {
            for(const auto& it2 : it.second) {
                std::cout << "Board: " << it.first << " channel: " << it2.first
                          << " has name: " << it2.second.name << std::endl;
            }
        }
    }
}

void SauronLut::PrintLut(std::ostream& outstream) {
    assert(!fLut_file_fullname.empty());
    std::ifstream old_lut_file(fLut_file_fullname.c_str());

    if(!old_lut_file.is_open())
        throw std::runtime_error("SAURON Lut " + fLut_file_fullname
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
        outstream << spaces[iter] << params.radius / UNITS::mm;
        ++iter;
        outstream << spaces[iter] << params.phi / UNITS::deg;
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
