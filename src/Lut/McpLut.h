#pragma once

#include <array>
#include <set>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TMatrixD.h>
#include <TVector3.h>

#include "Lut.h"
#include "Settings.h"

class ConfManager;

class McpLut : public Lut {
    friend class McpConf;
    friend class ConfManager;

  public:
    McpLut() : settings(Settings::getInstance()){};
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    enum McpType {
        X,
        Y,
        TOF12,
        TOF1DANTE,
        TOF2DANTE,
        XC,
        YC,
        TOF12C
    };
    constexpr static unsigned int lenMcpType{8};

    
    std::unordered_map<unsigned int, unsigned int> channelIdtoMcpId;
    std::unordered_map<unsigned int, McpType>    channelIdtoMcpType;
    std::unordered_map<unsigned int, std::array<unsigned int, 8>>
         mcpIdToChannelId;
    std::set<unsigned int> mcpIdSet;
    void SetRotation(const double&);

  private:
    std::unordered_map<unsigned int, double>                    timeOffset;
    std::unordered_map<unsigned int, std::pair<double, double>> thr;
    std::unordered_map<unsigned int, std::array<double, 2>>     tofCal{};

    struct Properties {
        // 3D 3x3
        TMatrixD                                       pcad;
        TMatrixD                                       p1;
        // 2D 3x3 but with one fictious dimension      
        TMatrixD                                       pdata;
        // Transformation to bring in 3D space     
        TMatrixD                                       tsf;
        std::array<bool, lenMcpType>                   tofsPresent{false};
        Properties() : tsf(4,3){};
    };
    std::unordered_map<unsigned int, Properties> prop;

    // The position is reconstructed with an affine transformation y = M x + b
    // (R^2 -> R^3) where y is the 3D position and x is the 2D values of mcp
    // (x-y). The affine transformation has been linearized by adding one
    // dimension to the matrices so that y' = M' x'. The matrix M' can be found
    // with M' = (y' x'^T)(x' x'^T)^-1.
    inline TVector3 getPos(const double& x, const double& y,
                           const unsigned int& mcpId) const {
        try {
            auto&    p = prop.at(mcpId);
            TMatrixD inputVec{3, 1};
            inputVec[0][0] = x;
            inputVec[1][0] = y;
            inputVec[2][0] = 1;

            auto res       = p.tsf * inputVec;
       //     std::cout<<res[0][0]<<" "<<res[1][0]<<" "<<res[2][0]<<std::endl;
            return {res[0][0], res[1][0], res[2][0]};
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Problem with the transformation matrix of MCP"
                                        +std::to_string(mcpId)+"\n");
        }
    }
    inline double getAvgCadX(const unsigned int& mcpId) const {
        const auto& pdata = prop.at(mcpId).pdata;
        int sum = 0;
        for(int ps{0}; ps<pdata.GetNcols(); ps++)
            sum += pdata[0][ps];    
        return sum/pdata.GetNcols();
    };
    inline double getAvgCadY(const unsigned int& mcpId) const {
        const auto& pdata = prop.at(mcpId).pdata;
        int sum = 0;
        for(int ps{0}; ps<pdata.GetNcols(); ps++)
            sum += pdata[1][ps];    
        return sum/pdata.GetNcols();
    };

    static inline unsigned int
    getChannelId(const unsigned int& brd, const unsigned int& chan) {
        return brd * Lut::MAXCH + chan;
    }
    inline unsigned int
    getMcpId(const unsigned int& brd, const unsigned int& chan) const {
        try {
            return channelIdtoMcpId.at(getChannelId(brd, chan));
        } catch(std::out_of_range& e) {
            throw std::runtime_error("getMcpId. Missing channel " + std::to_string(chan)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }
    inline std::pair<unsigned int, unsigned int>
    getBrdChanFromId(const unsigned int& mcpId,
                     const McpType&    type) const {
        try {
            return {mcpIdToChannelId.at(mcpId)[type] / Lut::MAXCH,
                    mcpIdToChannelId.at(mcpId)[type] % Lut::MAXCH};
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Something wrong in mcp lut?\n");
        }
    }
    inline McpType
    getMcpType(const unsigned int& brd, const unsigned int& chan) const {
        try {
            return channelIdtoMcpType.at(getChannelId(brd, chan));
        } catch(std::out_of_range& e) {
            throw std::runtime_error("getMcpType. Missing channel " + std::to_string(chan)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }
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
    getTof(const double& val, const unsigned int& brd, const unsigned int& chan) const {
        auto res = tofCal.find(getChannelId(brd, chan));
        if(res == tofCal.end())
            throw std::runtime_error("getTof. Missing channel " + std::to_string(chan)
                     + " in board " + std::to_string(brd)
                     + " in lookup table?\n");
        return res->second[0] + val * res->second[1];
    }


    inline bool
    hasTofCal(const unsigned int& brd, const unsigned int& chan) const {
        auto res = tofCal.find(getChannelId(brd, chan));
        if(res == tofCal.end())
            throw std::runtime_error("hasTofCal. Missing channel " + std::to_string(chan)
                     + " in board " + std::to_string(brd)
                     + " in lookup table?\n");
        if(res->second[0]!=0 && res->second[1]!=1) return true; 
        else return false;
    }

    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& chan) const {
        auto res = thr.find( getChannelId(brd, chan));
        if(res == thr.end())
            throw std::runtime_error("getThr. Missing channel " + std::to_string(chan)
                     + " in board " + std::to_string(brd)
                     + " in lookup table?\n");
        return res->second;
    }
    std::vector<unsigned int> getAllMcpId() const {
        std::vector<unsigned int> ids;
        ids.reserve(prop.size());
        for(const auto& it : prop) ids.emplace_back(it.first);
        return ids;
    }
    std::array<bool, lenMcpType> getTofsPresent(const unsigned int& mcpId) const {
        auto res = prop.find(mcpId);
        if(res == prop.end())
            throw std::runtime_error("getTofsPresent. Missing mcpId " + std::to_string(mcpId)
                                     + " in lookup table?\n");
        return res->second.tofsPresent;
    }

    const Settings& settings;
};
