#pragma once

#include <array>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TMatrixD.h>
#include <TVector3.h>

#include "Lut.h"
#include "Settings.h"
#include "Dante.h"

class ConfManager;

class DanteLut : public Lut {
    friend class DanteConf;
    friend class ConfManager;

  public:
    DanteLut() : settings(Settings::getInstance()){};
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    enum DanteType {
        X,
        Y,
        T,
        TOF,
    };
    //enum TofType { PRISMA, DANTE, NONE };
    std::unordered_map<unsigned int, unsigned int> channelIdtoDanteId;
    std::unordered_map<unsigned int, DanteType>    channelIdtoDanteType;
    std::unordered_map<unsigned int, std::array<unsigned int, 4>>
         danteIdToChannelId;
    void SetRotation(const double&);

  private:
    struct Properties {
        // 3D 3x3
        TMatrixD                                 pcad;
        TMatrixD                                 p1;
        // 2D 3x3 but with one fictious dimension
        TMatrixD                                 pdata;
        // Transformation to bring in 3D space
        TMatrixD                                 tsf;
        std::array<std::pair<double, double>, 4> thr;
        std::array<double, 2>                    tofCal{};
        Dante::TofType                           tofType{Dante::TofType::NONE};
        Properties() : tsf(4, 3){};
    };
    std::unordered_map<unsigned int, Properties> prop;
    std::unordered_map<unsigned int, double>     timeOffset;

    // The position is reconstructed with an affine transformation y = M x + b
    // (R^2 -> R^3) where y is the 3D position and x is the 2D values of dante
    // (x-y). The affine transformation has been linearized by adding one
    // dimension to the matrices so that y' = M' x'. The matrix M' can be found
    // with M' = (y' x'^T)(x' x'^T)^-1.
    inline TVector3 getPos(const double& x, const double& y,
                           const unsigned int& danteId) const {
        try {
            auto&    p = prop.at(danteId);
            TMatrixD inputVec{3, 1};
            inputVec[0][0] = x;
            inputVec[1][0] = y;
            inputVec[2][0] = 1;

            auto res       = p.tsf * inputVec;
            return {res[0][0], res[1][0], res[2][0]};
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing dante id "
                                     + std::to_string(danteId)
                                     + " in lookup table?\n");
        }
    }
    inline double getAvgX(const unsigned int& danteId) const {
        const auto& pdata = prop.at(danteId).pdata;
        // 0 and 2 are averaged first  because they should correspond roughly
        // to the same position
        return ((pdata[0][0] + pdata[0][2]) / 2 + pdata[0][1]) / 2;
    };
    inline double getAvgY(const unsigned int& danteId) const {
        const auto& pdata = prop.at(danteId).pdata;
        // 0 and 1 are averaged first  because they should correspond roughly
        // to the same position
        return ((pdata[1][0] + pdata[1][1]) / 2 + pdata[1][2]) / 2;
    };
    inline double
    getTime(const double& val, const unsigned int& brd, const unsigned int& chan) const {
        auto res = timeOffset.find(getChannelId(brd, chan));
        if(res == timeOffset.end())
            throw std::runtime_error("getTime Missing channel " + std::to_string(chan)
                     + " in board " + std::to_string(brd)
                     + " in lookup table?\n");
        return val + res->second;
    }
    inline double
    getTof(const double& val, const unsigned int& danteId) const {
        try {
            auto& p = prop.at(danteId).tofCal;
            return p[0] + val * p[1];
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing dante id "
                                     + std::to_string(danteId)
                                     + " in lookup table?\n");
        }
    }
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& chan) const {
        try {
            unsigned int id = getChannelId(brd, chan);
            return prop.at(channelIdtoDanteId.at(id))
                .thr[channelIdtoDanteType.at(id)];
        } catch(std::out_of_range& e) {
            throw std::runtime_error("getThr Missing channel " + std::to_string(chan)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }
    static inline unsigned int
    getChannelId(const unsigned int& brd, const unsigned int& chan) {
        return brd * Lut::MAXCH + chan;
    }
    inline unsigned int
    getDanteId(const unsigned int& brd, const unsigned int& chan) const {
        try {
            return channelIdtoDanteId.at(getChannelId(brd, chan));
        } catch(std::out_of_range& e) {
            throw std::runtime_error("getDanteId Missing channel " + std::to_string(chan)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }
    inline std::pair<unsigned int, unsigned int>
    getBrdChanFromId(const unsigned int& danteId,
                     const DanteType&    type) const {
        try {
            return {danteIdToChannelId.at(danteId)[type] / Lut::MAXCH,
                    danteIdToChannelId.at(danteId)[type] % Lut::MAXCH};
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Something wrong in dante lut?\n");
        }
    }
    inline DanteType
    getDanteType(const unsigned int& brd, const unsigned int& chan) const {
        try {
            return channelIdtoDanteType.at(getChannelId(brd, chan));
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(chan)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }
    inline bool hasTof(const unsigned int& danteId) const {
        return prop.find(danteId)->second.tofType != Dante::TofType::NONE;
    }
    inline bool tofType(const unsigned int& danteId) const {
        return prop.find(danteId)->second.tofType;
    }
    std::vector<unsigned int> getAllDanteId() const {
        std::vector<unsigned int> ids;
        ids.reserve(prop.size());
        for(const auto& it : prop) ids.emplace_back(it.first);
        return ids;
    }
    // inline unsigned long long getTime(const unsigned int& val, const
    // unsigned int& danteId) const{
    //     try{
    //         auto ret =  val + prop.at(danteId).timeOffset;
    //         return ret;
    //     }catch (std::out_of_range& e){
    //         throw std::runtime_error("Missing channel in lookup table?\n");
    //     }
    // }
    const Settings& settings;
};
