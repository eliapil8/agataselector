#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TVector3.h>

#include "Lut.h"

class ConfManager;

class MonitorLut : public Lut {
    friend class MonitorConf;
    friend class ConfManager;

  public:
    MonitorLut() = default;
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    struct Properties {
        std::vector<double>       cal;
        TVector3                  pos;
        TVector3                  posOrig;
        double                    timeOffset;
        std::pair<double, double> thr;
        std::string               name;
        unsigned int              map;
    };

  private:
    std::string fLut_file_fullname;
    std::unordered_map<unsigned int,
                       std::unordered_map<unsigned int, Properties>>
        prop;

  public:
    std::unordered_map<unsigned int,
                       std::unordered_map<unsigned int, Properties>>&
    GetProperties() {
        return prop;
    };

    inline double getEnergy(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        double en{0};
        try {
            const std::vector<double>& c = prop.at(brd).at(ch).cal;
            for(unsigned int i{0}; i < c.size(); ++i) {
                en += c[i] * pow(val, i);
            }
            return en;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    std::vector<unsigned int> getAllMonitorId() const {
        std::vector<unsigned int> ids;
        ids.reserve(prop.size());
        for(const auto& it : prop.begin()->second) {
            unsigned int monNr;
            std::string chanName = it.second.name; 
            monNr = static_cast<unsigned int>(   
            std::stoi(chanName.substr(chanName.find_first_not_of('M'),
                                      chanName.find_first_not_of("M")-chanName.size())));
            ids.emplace_back(monNr);
        }
        return ids;
    }

    inline double getTime(const double& val, const unsigned int& brd,
                          const unsigned int& ch) const {
        try {
            auto ret = val + prop.at(brd).at(ch).timeOffset;
            return ret;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }

    inline TVector3
    getPos(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).pos;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }

    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).thr;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }

    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).name;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };
};
