#pragma once

#include "CoincidenceConf.h"
#include "DetectorAnalysis/DetectorAnalysis.h"
#include "DetectorConf.h"

class CoincidenceAnalysis : public DetectorAnalysis {
  public:
    CoincidenceAnalysis() = delete;

  protected:
    struct CoincidenceHistograms : public DetectorHistograms {
        TH1D* h_TSmTS{nullptr};
        TH2D* m_TSmTS_TS{nullptr};
        TH1D* h_TSmTSgated{nullptr};
    };

  protected:
    CoincidenceAnalysis(Inputs&, Container&, DetectorConf&, Container&,
                        DetectorConf&, CoincidenceConf&, Readers&, Readers&,
                        CoincidenceHistograms&);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    Container&             data2;
    DetectorConf&          detectorConf2;
    CoincidenceConf&       coincidenceConf;
    CoincidenceHistograms& coincidenceHistograms;
    std::vector<std::vector<bool>>  coincMatrix;
    bool in_coinc{false};

    Readers&               readers1;
    Readers&               readers2;
};
