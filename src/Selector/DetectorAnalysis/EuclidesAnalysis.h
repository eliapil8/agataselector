#pragma once

#include "DetectorAnalysis.h"
#include "Euclides.h"

class EuclidesAnalysis : public DetectorAnalysis
{
  public:
    explicit EuclidesAnalysis(Inputs, Euclides &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Euclides &euclides;

    // Histograms
    struct EuclidesHistograms : public DetectorHistograms
    {
        TH1D *h_mult{nullptr};
        TH1D *h_mon{nullptr};
        TH2D *m_EuclidesID_timediff_dE{nullptr};
        TH2D *m_EuclidesID_timediff_E{nullptr};
        TH2D *m_Energy_TS{nullptr};
        TH2D *m_Theta_Phi{nullptr};
        TH2D *m_layout_XY{nullptr};
        TH2D *m_layout_XZ{nullptr};
        TH2D *m_layout_YZ{nullptr};
        TH2D *m_Energy_Theta{nullptr};
        TH2D *m_Energy_euclidesID{nullptr};
        TH1D *h_Ex{nullptr};
        TH2D *m_E_dE{nullptr};
        struct EDE : public HistogramStructure
        {
            std::unordered_map<unsigned int, TH2D *> m_E_dE;
        } ede;
    } euclidesHistograms;

    // Readers
  public:
    struct EuclidesReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbEUCLIDES{nullptr};
        TTreeReaderValue<ULong64_t> *TSEUCLIDES{nullptr};
        TTreeReaderArray<ULong64_t> *TSEUCLIDES_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *EUCLIDESChannel{nullptr};
        TTreeReaderArray<UShort_t>  *EUCLIDESBoard{nullptr};
        TTreeReaderArray<Float_t>   *EUCLIDESEnergy{nullptr};
        TTreeReaderArray<Float_t>   *EUCLIDESTime{nullptr};
    } euclidesReader;
};
