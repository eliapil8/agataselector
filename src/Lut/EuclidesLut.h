#pragma once

#include <cassert>
#include <exception>
#include <string>
#include <unordered_map>
#include <vector>

#include <TCutG.h>
#include <TVector3.h>

#include "Lut.h"
#include "ReactionFragment.h"

class ConfManager;

class EuclidesLut : public Lut {
    friend class EuclidesConf;
    friend class ConfManager;

  public:
    EuclidesLut() = default;
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    void LoadCuts(const std::vector<TCutG*>&);

  private:
    struct Properties {
        unsigned int                                          map_detID;
        std::vector<double>                                   cal;
        TVector3                                              pos;
        TVector3                                              posOrig;
        double                                                timeOffset;
        std::pair<double, double>                             thr;
        std::string                                           name;
        bool                                                  isE{false};
        bool                                                  isdE{false};
        unsigned int                                          ring;
        unsigned int                                          det;
        unsigned int                                          seg;
        std::vector<std::pair<ReactionFragment::Ion, TCutG*>> pId;
    };

    std::string fLut_file_fullname;

    std::unordered_map<BoardChan, Properties, BoardChan::hash_fn> prop;

    inline double getEnergy(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        double en{0};
        try {
            const std::vector<double>& c = prop.at(BoardChan(brd, ch)).cal;
            for(unsigned int i{0}; i < c.size(); ++i) {
                en += c[i] * pow(val, i);
            }
            return en;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    };

    inline double getTime(const double& val, const unsigned int& brd,
                          const unsigned int& ch) const {
        try {
            auto ret = val + prop.at(BoardChan(brd, ch)).timeOffset;
            return ret;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }

    inline TVector3
    getPos(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).pos;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }

    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).thr;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }

    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).name;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    };
    inline unsigned int
    getMap(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).map_detID;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    };
    inline bool isE(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).isE;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }
    inline bool isdE(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).isdE;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }
    inline unsigned int
    getRing(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).ring;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }
    inline unsigned int
    getDet(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).det;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }
    inline unsigned int
    getSeg(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(BoardChan(brd, ch)).seg;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " on board " + std::to_string(brd)
                                     + " in lookup table? \n");
        }
    }
    inline ReactionFragment::Ion
    idParticle(const double& e, const double& de, const unsigned int& brd,
               const unsigned int& ch) const {
        try {
            ReactionFragment::Ion ion(0, 0);
            bool                  alreadyFound{false};
            for(const auto& it : prop.at(BoardChan(brd, ch)).pId) {
                if(it.second->IsInside(e, de)) {
                    if(!alreadyFound) {
                        alreadyFound = true;
                        ion          = it.first;
                    } else {
                        throw std::runtime_error(
                            std::string("Overlapping EUCLIDES gates: id ")
                            + it.second->GetName() + "\n");
                    }
                }
            }
            return ion;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel in lookup table?\n");
        }
    }
};
