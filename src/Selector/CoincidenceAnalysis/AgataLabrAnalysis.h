#pragma once

#include "Agata.h"
#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/LabrAnalysis.h"
#include "Labr.h"

class AgataLabrAnalysis : public CoincidenceAnalysis {
  private:
    struct AgataLabrHistograms : public CoincidenceHistograms {
        TH2D* m_TSdiff_TS{nullptr};
        TH1D* h_cTime_gated{nullptr};
        // TH1D* h_TSlabrmTSagata{nullptr};
        TH2D* m_TSlabrmTSagata_labrID{nullptr};
        TH1D* h_Eaga{nullptr};
        TH1D* h_Elabr{nullptr};
        TH2D* m_EE_labr{nullptr};
        TH2D* m_Eaga_Elabr{nullptr};
        TH2D* m_Eaga_LabrTdiff{nullptr};
    } agataLabrHistograms;

  public:
    AgataLabrAnalysis(Inputs&, Agata&, Labr&, AgataAnalysis::AgataReader&,
                      LabrAnalysis::LabrReader&);
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Agata&                      agata;
    Labr&                       labr;
    AgataAnalysis::AgataReader& agataReader;
    LabrAnalysis::LabrReader&   labrReader;
};
