#pragma once

#include "Dante.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorPrismaAnalysis.h"
#include "GammadetectorPrismaConf.h"
#include "Prisma.h"
#include "GateSelector.h"

class GammadetectorPrismaDanteAnalysis : public GammadetectorPrismaAnalysis {
  protected:
    struct GammadetectorPrismaDanteHistograms
        : public GammadetectorPrismaHistograms {
        TH2D* m_TSprismamTSgamma_TSdantemTSgamma{nullptr};
        struct tofTKELHistograms
            : public GammadetectorPrismaHistograms::IonHistograms {
            //TCutG* cut;
            //explicit tofTKELHistograms(TCutG* cut) : cut(cut){};
        };
        std::unordered_map<int,
                           std::unordered_map<ReactionFragment::Ion, tofTKELHistograms,
                           ReactionFragment::Ion::hash_fn>>
                                       tofTKELIon;
    };

  protected:
    GammadetectorPrismaDanteAnalysis(Inputs&, GammadetectorPrismaConf&,
                                     GammadetectorDanteConf&, Gammadetector&,
                                     GammadetectorConf&, Prisma&, Dante&,
                                     Readers&, PrismaAnalysis::PrismaReader&,
                                     DanteAnalysis::DanteReader&, GammaDC&,
                                     GammadetectorPrismaDanteHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Dante&                              dante;
    GammadetectorDanteConf&             gammadetectorDanteConf;
    GammaDC&                            gammaDC;
    GammadetectorPrismaDanteHistograms& gammadetectorPrismaDanteHistograms;
    PrismaAnalysis::PrismaReader&       prismaReader;
    DanteAnalysis::DanteReader&         danteReader;

    GateSelector<1,1> TOF_QVAL_Gate;
};
