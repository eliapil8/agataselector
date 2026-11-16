#pragma once

#include <vector>

#include <TLorentzVector.h>
#include <TVector2.h>
#include <TVector3.h>

#include "iostream"

class GammaDC : public TObject {
  public:
    std::vector<bool>           inCoinc;
    std::vector<TLorentzVector> E;
    std::vector<TLorentzVector> EDC;
    std::vector<TLorentzVector> EDCBP;
    TLorentzVector              p3;
    TLorentzVector              p4;

    void                        clear() {
        inCoinc.clear();
        E.clear();
        EDC.clear();
        EDCBP.clear();
        p3 = TLorentzVector();
        p4 = TLorentzVector();
    }
    virtual void resize(const unsigned int& mult) {
        inCoinc.resize(mult);
        E.resize(mult);
        EDC.resize(mult);
        EDCBP.resize(mult);
    };

    class redGammaDC : public TObject {
      public:
        std::vector<bool>  rinCoinc;
        std::vector<float> rE;
        std::vector<float> rEDC;
        std::vector<float> rEDCBP;

        void                        clear() {
            rinCoinc.clear();
            rE.clear();
            rEDC.clear();
            rEDCBP.clear();
        }
        virtual void resize(const unsigned int& mult) {
            rinCoinc.resize(mult);
            rE.resize(mult);
            rEDC.resize(mult);
            rEDCBP.resize(mult);
        };
        
        ClassDef(redGammaDC, 4);

    } redgammaDC;

    ClassDef(GammaDC, 4);
};
