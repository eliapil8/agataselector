#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TVector3.h>

#include "Lut.h"

// bool operator==(const std::pair<unsigned int, unsigned int>& l, const
// std::pair<unsigned int, unsigned int>& r) { return l.first == r.first &&
// l.second == r.second; };

class ConfManager;

class GammadetectorSpiderLut : public Lut {
    friend class GammadetectorSpiderConf;
    friend class ConfManager;

  public:
    GammadetectorSpiderLut() = default;
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;

  private:
    struct hash_fn {
        std::size_t
        operator()(const std::pair<unsigned int, unsigned int>& p) const {
            return std::hash<unsigned int>()(p.first * 128)
                 ^ std::hash<unsigned int>()(p.second);
        }
    };

    typedef std::unordered_map<std::pair<unsigned int, unsigned int>,
                               std::pair<double, double>, hash_fn>
                        ExGate;

    std::vector<ExGate> exGate;

    inline bool
    isDeclared(const unsigned int& gatenum, const unsigned int& board,
               const unsigned int& channel) {
        auto& it      = exGate.at(gatenum);
        auto  element = it.find(std::make_pair(board, channel));
        if(element == it.end()) return false;
        return true;
    }

    inline bool
    isInside(const unsigned int& gatenum, const unsigned int& board,
             const unsigned int& channel, const double& Ex) {
        auto& it = exGate.at(gatenum);
        // auto& it2 = it.find(std::make_pair(floor(id/16), id%16));
        if(Ex > it[std::make_pair(board, channel)].first
           && Ex < it[std::make_pair(board, channel)].second)
            return true;
        else
            return false;
    }

    /*    inline std::pair<double, double> getGate(const unsigned int& gatenum,
    const unsigned int& board, const unsigned int& channel) const { auto &it =
    exGate.at(gatenum)->gate; auto &it2 = it.find(std::make_pair(board,
    channel)); if(it2 != it.end()) //we need to  check if this thing exists
    return it.at(it2); else { std::pair<double, double> fakeThr = {0., 100.};
    return fakeThr;
        }
    };
    */
};
