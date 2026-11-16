#pragma once

#include <array>

#include "McpLut.h"
#include "DetectorConf.h"
#include "ReactionFragment.h"
#include "TFile.h"
#include "TCutG.h"

class McpConf : public DetectorConf {
public:
  McpConf();
  
  ReactionFragment::Ion ion;
  bool                                mandatoryX{true};
  bool                                mandatoryY{true};
  bool                                mandatoryTOF{false};
  bool                                debug{false};
  bool                                enablecuttof{false};
  bool                                enableCalcuttof{false};
  bool                                enablecutxy{false};
  std::vector<std::pair<int,int>>     indexgates_xtof;//{{0,2}};
  std::vector<int>                    indexgates_xy;
  std::vector<double>                 offx;
  std::vector<double>                 offy;
  std::vector<double>                 offz;
  int                                 nDetectors{2};
  double                              tolerance{2};
  double                              bline1radius{50};
  double                              bline2radius{25};
  double                              bline1zmin{-2066.9800};
  double                              bline1zmax{-1079.9800};
  double                              bline2zmin{-1079.9800};
  double                              bline2zmax{-172};
  double                              avgx1{0};
  double                              avgx2{0};
  double                              avgy1{0};
  double                              avgy2{0};
  double                              sourcex{-0.1424};
  double                              sourcey{-14.0124};
  double                              sourcez{-3394.0};
  double                              dcentremcps{1088.32};
  double                              dcentremcp1{-3382.93};
  McpLut                lut;
  
   
    inline std::pair<double, double>
    getThr(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getThr(brd, ch);
    }
    inline unsigned int
    getMcpId(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getMcpId(brd, ch);
    }
    inline std::pair<unsigned int, unsigned int>
    getBrdChanFromId(const unsigned int&        mcpId,
                     const McpLut::McpType& type) const {
        return lut.getBrdChanFromId(mcpId, type);
    }
    inline unsigned int
    getMcpType(const unsigned int& brd, const unsigned int& ch) const {
        return lut.getMcpType(brd, ch);
    }
    inline TVector3 getPosition(const double& x, const double& y,
                                const unsigned int& mcpId) const {
        TVector3 vec = lut.getPos(x, y, mcpId);
        vec += TVector3(xShift, yShift, zShift);
        vec.RotateZ(phi);
        vec.RotateX(thetaX);
        vec.RotateY(thetaY);


	    vec.SetX(vec.X()+offx.at(mcpId-1));
	    vec.SetY(vec.Y()+offy.at(mcpId-1));
	    vec.SetZ(vec.Z()+offz.at(mcpId-1));
        return vec;
    }
    inline double getAvgCadX(const unsigned int& mcpId) const {
        return lut.getAvgCadX(mcpId);
    }
    inline double getAvgCadY(const unsigned int& mcpId) const {
        return lut.getAvgCadY(mcpId);
    }
    inline double getAvgX(const unsigned int& mcpId) const {
            double val{0};
            if(mcpId == 1) val = avgx1;
            else if(mcpId == 2) val = avgx2;
            else{
            throw std::runtime_error("Missing mcp id "
                                     + std::to_string(mcpId)
                                     + " in conf file?\n");
            }
            return val;
    }
    inline double getAvgY(const unsigned int& mcpId) const {
            double val{0};
            if(mcpId == 1) val = avgy1;
            else if(mcpId == 2) val = avgy2;
            else{
            throw std::runtime_error("Missing mcp id "
                                     + std::to_string(mcpId)
                                     + " in conf file?\n");
            }
            return val;
    }
    inline double
    getTime(const double& val, const unsigned int& brd, const unsigned int& chan) const {
        return lut.getTime(val, brd, chan);
    }
    inline double
    getTof(const double& val, const unsigned int& brd, const unsigned int& chan) const {
        return lut.getTof(val, brd, chan);
    }
    inline bool hasTofCal(const unsigned int& brd, const unsigned int& chan) const {
       return lut.hasTofCal(brd, chan);
    }
   // inline bool tofType(const unsigned int& mcpId) const {
   //     return lut.tofType(mcpId);
   // }
    std::vector<unsigned int> getAllMcpId() const {
        return lut.getAllMcpId();
    };
    std::array<bool, McpLut::lenMcpType> getTofsPresent(const unsigned int& mcpId) const {
        return lut.getTofsPresent(mcpId);
    }
};
