#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TCutG.h>
#include <TVector3.h>

#include "Lut.h"
#include "ReactionFragment.h"
#include "Settings.h"

class ConfManager;

class SauronLut : public Lut {
    friend class SauronConf;
    friend class ConfManager;

  public:
    SauronLut();
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    void LoadCuts(const std::vector<TCutG*>&);
    // void generatePositions(const double&, const double&, const double&);
    enum SauronSide { PSIDE, NSIDE };

  private:
    const Settings& settings;
    struct Properties {
        SauronSide                                            side;
        int                                                   segment;
        std::string                                           name;
        std::pair<double, double>                             thr;
        double                                                radius;
        double                                                phi;
        double                                                timeOffset;
        std::vector<double>                                   cal;
        unsigned int                                          map;
    };
    std::string fLut_file_fullname;

    std::unordered_map<unsigned int,
                       std::unordered_map<unsigned int, Properties>>
                  prop;

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
    };

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

    inline double
    getRadius(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).radius;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline double
    getPhi(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).phi;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline bool
    isPside(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return ((prop.at(brd).at(ch).side == SauronSide::PSIDE) ? true
                                                                    : false);
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline bool
    isNside(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return ((prop.at(brd).at(ch).side == SauronSide::NSIDE) ? true
                                                                    : false);
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline int
    getSegment(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(brd).at(ch).segment;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline bool
    isPresent(const unsigned int& brd, const unsigned int& ch) const {
        if(prop.find(brd) != prop.end()) {
            if(prop.at(brd).find(ch) != prop.at(brd).end()) return true;
        }
        return false;
    };
};
