#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TCutG.h>
#include <TVector3.h>
#include <TMatrixD.h>


#include "Lut.h"
#include "ReactionFragment.h"
#include "Settings.h"

class ConfManager;

class EXSSiDeLut : public Lut {
    friend class EXSSiDeConf;
    friend class ConfManager;

  public:
    EXSSiDeLut();
    void ReadLut(const std::string&) override;
    void PrintLut(std::ostream& outstream = std::cout) override;
    enum EXSSiDeSide { PSIDE, NSIDE };
  private:
    const Settings& settings;
    struct Properties {
        EXSSiDeSide                                            side;
        int                                                   strip;
        std::string                                           name;
        std::pair<double, double>                             thr;
        double                                                width;
        double                                                timeOffset;
        std::vector<double>                                   cal;
        unsigned int                                          map;
        unsigned int                                          brd;
       // std::vector<std::pair<ReactionFragment::Ion, TCutG*>> pId;
    };
    std::string fLut_file_fullname;
    TMatrixD    pcad;
    TMatrixD    pdata;
    // Transformation to bring in 3D space
    TMatrixD    tsf;    
    // channel to X, Y transformation 
    //std::unordered_map<unsigned int,std::unordered_map<unsigned int, std::pair<double,double>>> mapChanIdtoPos;
    std::unordered_map<unsigned int,Properties>
                  prop;

    inline double getEnergy(const double& val, const unsigned int& brd,
                            const unsigned int& ch) const {
        double en{0};
        try {
            const std::vector<double>& c = prop.at(ch).cal;
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
            //std::cout<<"Time "<<val<<std::endl;
            auto ret = val + prop.at(ch).timeOffset;
           //std::cout<<"Time Ofss "<<ret<<std::endl;
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
            return prop.at(ch).thr;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    }

    inline std::string
    getName(const unsigned int& brd, const unsigned int& ch) const {
        try {
            //std::cout<<"Name ch "<<ch<<" "<<prop.at(ch).name<<std::endl;
            return prop.at(ch).name;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline double
    getWidth(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return prop.at(ch).width;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline bool
    isPside(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return ((prop.at(ch).side == EXSSiDeSide::PSIDE) ? true: false);
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline bool
    isNside(const unsigned int& brd, const unsigned int& ch) const {
        try {
            return ((prop.at(ch).side == EXSSiDeSide::NSIDE) ? true
                                                                    : false);
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in board " + std::to_string(brd)
                                     + " in lookup table?\n");
        }
    };

    inline int
    getStripNr(const unsigned int& ch) const {
        try {
           // std::cout<<"StripN ch "<<ch<<" "<<prop.at(ch).strip<<std::endl;
            return prop.at(ch).strip;
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Missing channel " + std::to_string(ch)
                                     + " in lookup table?\n");
        }
    };

    inline unsigned int
    getChannelId(const int& stripn) const {
    
        unsigned int ch{999};
        bool found{false};
        for(auto it : prop){
            if(it.second.strip == stripn){
                found = true;
                ch = it.first;
                break;
            } 
        }
        if(!found){
            throw std::runtime_error("Missing strip " + std::to_string(stripn)
                                     + " in board\n"); 
        }
        return ch;
        
    };

    inline bool
    isPresent(const unsigned int& brd, const unsigned int& ch) const {
        if(prop.find(ch) != prop.end()) {
            if(prop.find(brd) != prop.end()) return true;
        }
        return false;
    };

    inline std::pair<double,double> mapChanIdtoPos (const unsigned int &ch1, const unsigned int &ch2) const{

        std::unordered_map<unsigned int,std::unordered_map<unsigned int, std::pair<double,double>>> chToPos;
         // from stripNr to position 2D    
        // depending on which side is horizontal and which is vertical
        // I assume NSIDE horizontal strip. So its vertical coord
        
        for(unsigned int k{0}; k<32; k++)
            for(unsigned int j{0}; j<32; j++)
                chToPos[k][j] = {-31+ k*prop.at(k).width,-31+j*prop.at(j+32).width};
        //for(unsigned int k{0}; k<32; k++)
        //    for(unsigned int j{0}; j<16; j++)
        //        chToPos[k][j] = {-31+ k*prop.at(k).width,-31+j*prop.at(j+32).width};
        //for(unsigned int k{0}; k<32; k++)
        //    for(unsigned int j{16}; j<32; j++)
        //        chToPos[k][j] = {-31+ k*prop.at(k).width,63-j*prop.at(j+32).width};

        return {chToPos[ch1][ch2].first,chToPos[ch1][ch2].second};

    };

    inline TVector3 getPos(const unsigned int &ch1, const unsigned int &ch2) const {
        
        double ns, ps;
        if(isPside(prop.at(ch1).brd,ch1) and isNside(prop.at(ch2).brd, ch2)){
            ps = ch1;
            ns = ch2;
        }
        else if(isPside(prop.at(ch2).brd,ch2) and isNside(prop.at(ch1).brd, ch1)){
            ps = ch2;
            ns = ch1;
        }
        else{
            throw std::runtime_error("Error! Give one channel for PSIDE and one for NSIDE\n");
        }

        try {
            
            TMatrixD inputVec{3, 1};
            inputVec[0][0] = mapChanIdtoPos(getStripNr(ns),getStripNr(ps)).second;
            inputVec[1][0] = mapChanIdtoPos(getStripNr(ns),getStripNr(ps)).first;
            inputVec[2][0] = 1;

            auto res       = tsf * inputVec;
            return {res[0][0], res[1][0], res[2][0]};
        } catch(std::out_of_range& e) {
            throw std::runtime_error("Error in the transformation matrix of EXSSiDe\n");
        }
    };


};
