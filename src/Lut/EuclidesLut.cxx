#include "EuclidesLut.h"
#include "Settings.h"
#include "Units.h"
#include <algorithm>
#include <sstream>

void EuclidesLut::ReadLut(const std::string& file_name) {
    fLut_file_fullname = file_name;
    prop.clear();
    unsigned int  mapNumber;
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Euclides Lut not present\n");
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
        prop[BoardChan(board, chan)].pos.SetMagThetaPhi(
            90 * UNITS::mm, th * UNITS::deg, ph * UNITS::deg);
        prop[BoardChan(board, chan)].posOrig
            = prop[BoardChan(board, chan)].pos;
        prop[BoardChan(board, chan)].timeOffset = tO * 10 * UNITS::ns;
        prop[BoardChan(board, chan)].thr  = std::make_pair(thrLow, thrHigh);
        prop[BoardChan(board, chan)].name = chanName;
        prop[BoardChan(board, chan)].map_detID = mapNumber;

        for(unsigned int i{0}; i < nCal; ++i) {
            strline >> coeff;
            (prop[BoardChan(board, chan)].cal).push_back(coeff);
        }
    }

    for(auto& it : prop) {
        it.second.isE  = (it.second.map_detID / 1000) == 1;
        it.second.isdE = (it.second.map_detID / 1000) == 2;
        it.second.ring = (it.second.map_detID % 1000) / 100;
        it.second.seg  = (it.second.map_detID % 10);
    }

    if(Settings::getInstance().verbosity_level > 0) {
        std::cout << "Euclides lut " << std::endl;
        for(const auto& it : prop) {
            std::cout << "silicon ID : " << it.second.map_detID << " cal ";
            for(const auto& c : it.second.cal) { std::cout << c << " "; }
            std::cout << " timeoffset : "
                      << it.second.timeOffset / (10 * UNITS::ns) << " ns";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void EuclidesLut::LoadCuts(const std::vector<TCutG*>& cuts) {
    for(const auto& it : cuts) {
        std::string name = it->GetName();
        std::replace(name.begin(), name.end(), '.', '_');
        unsigned int A  = static_cast<unsigned int>(std::stoi(name.substr(
            name.find_first_of('a') + 1, name.find_first_of('_') - 1)));
        unsigned int Z  = static_cast<unsigned int>(std::stoi(name.substr(
            name.find_first_of('z') + 1, name.find_first_of('_') - 1)));
        unsigned int id = static_cast<unsigned int>(
            std::stoi(name.substr(name.find_last_of('_') + 1)));

        for(auto& p : prop) {
            if(p.second.map_detID == id) {
                p.second.pId.emplace_back(ReactionFragment::Ion(A, Z), it);
            }
        }
    }
    // Look for duplicates
    for(auto& it : prop) {
        std::sort(it.second.pId.begin(), it.second.pId.end());
        auto itUn = std::unique(
            it.second.pId.begin(), it.second.pId.end(),
            [](const auto& a, const auto& b) { return a.first == b.first; });
        bool wasUnique = (itUn == it.second.pId.end());
        if(!wasUnique)
            throw std::runtime_error(
                "Multiple gates for the same particle and detector\n");
    }

    if(Settings::getInstance().verbosity_level > 0) {
        std::cout << "Euclides cuts " << std::endl;
        for(const auto& it : prop) {
            std::cout << "Detector ID: " << it.second.map_detID
                      << " with name: " << it.second.name << std::endl;
            for(const auto& it2 : it.second.pId) {
                std::cout << "Ion A" << it2.first.A << " Z" << it2.first.Z
                          << " with name " << it2.second->GetName()
                          << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

void EuclidesLut::PrintLut(std::ostream& outstream) {
    assert(!fLut_file_fullname.empty());
    std::ifstream old_lut_file(fLut_file_fullname.c_str());

    if(!old_lut_file.is_open())
        throw std::runtime_error("Euclides Lut " + fLut_file_fullname
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
        auto         params = prop[BoardChan(board, chan)];

        // print until theta
        unsigned int iter   = 0;

        // sanity check
        assert(spaces.size() >= 10 + params.cal.size()
               && "not enough spaces in the line to print the lut");

        outstream << spaces[iter] << board;
        ++iter;
        outstream << spaces[iter] << chan;
        ++iter;
        outstream << spaces[iter] << params.map_detID;
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