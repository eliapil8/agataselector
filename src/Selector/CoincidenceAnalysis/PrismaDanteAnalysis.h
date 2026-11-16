#pragma once

#include "CoincidenceAnalysis.h"
#include "Dante.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "Prisma.h"

class PrismaDanteAnalysis : public CoincidenceAnalysis {
    struct PrismaDanteHistograms : public CoincidenceHistograms {
        TH2D* m_TSdiff_TS{nullptr};
        TH1D* h_cTime_gated{nullptr};
        // TH1D* h_TSdantemTSprisma{nullptr};
        TH2D* m_danteId_TSmTS{nullptr};
        TH2D* m_ThetaPrisma_ThetaDante{nullptr};
        TH2D* m_PhiPrisma_PhiDante{nullptr};
        struct TOF : public HistogramStructure {
            std::unordered_map<unsigned int, TH1D*> h_TOF;
            std::unordered_map<unsigned int, TH2D*> m_TOF_Z;
            std::unordered_map<unsigned int, TH2D*> m_TOF_E;
            std::unordered_map<unsigned int, TH2D*> m_TOF_Qval;
            std::unordered_map<unsigned int, TH2D*> m_TOF_beta;
            std::unordered_map<unsigned int, TH2D*> m_TSdiff_Z;
            std::unordered_map<unsigned int, TH2D*> m_TSdiff_E;
            std::unordered_map<unsigned int, TH2D*> m_TSdiff_Qval;
        } tof;
        struct IonHistograms : public HistogramStructure {
            TH2D* m_ThetaPrisma_ThetaDante{nullptr};
            TH2D* m_PhiPrisma_PhiDante{nullptr};
            TOF   tof;
        };
        std::unordered_map<ReactionFragment::Ion, IonHistograms,
                           ReactionFragment::Ion::hash_fn>
            ionHistos;
    } prismaDanteHistograms;

  public:
    PrismaDanteAnalysis(Inputs&, Prisma&, Dante&,
                        PrismaAnalysis::PrismaReader&,
                        DanteAnalysis::DanteReader&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Prisma&                       prisma;
    Dante&                        dante;
    PrismaAnalysis::PrismaReader& prismaReader;
    DanteAnalysis::DanteReader&   danteReader;

  private:
};
