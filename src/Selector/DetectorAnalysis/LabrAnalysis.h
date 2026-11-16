#pragma once

#include "DetectorAnalysis.h"
#include "Labr.h"

#include <TRandom3.h>

class LabrAnalysis : public DetectorAnalysis
{
  public:
    explicit LabrAnalysis(Inputs, Labr &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Labr &labr;

    // Histograms
    struct LabrHistograms : public DetectorHistograms
    {
        TH2D *m_layout_XY{nullptr};
        TH2D *m_layout_YZ{nullptr};
        TH2D *m_layout_XZ{nullptr};
        TH3D *m_layout_XYZ{nullptr};
        TH1D *h_E_labr_raw{nullptr};
        TH2D *m_E_labr_raw_ID{nullptr};
        TH1D *h_E_labr{nullptr};
        TH2D *m_EE_labr{nullptr};
        TH2D *m_E_Tdiff{nullptr};
        TH2D *m_Energy_TS{nullptr};
        TH1D *h_monitor{nullptr};
        TH2D *m_labrID_tdiff{nullptr};
        struct LabrAnaHistograms : public HistogramStructure
        {
            TH1D *h_EDC_avg_labr{nullptr};
            TH2D *m_E_labrID{nullptr};
            TH2D *m_EDC_EDC_avg_labr{nullptr};
            TH2D *m_Theta_E_avg_labr{nullptr};
            TH2D *m_Theta_EDC_avg_labr{nullptr};
            TH2D *m_montdiff_EDClabr{nullptr};
        } ana;
    } labrHistograms;

    // Readers
  public:
    struct LabrReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbLABR{nullptr};
        TTreeReaderValue<ULong64_t> *TSLABR{nullptr};
        TTreeReaderArray<ULong64_t> *TSLABR_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *LABRChannel{nullptr};
        TTreeReaderArray<UShort_t>  *LABRBoard{nullptr};
        TTreeReaderArray<Float_t>   *LABREnergy{nullptr};
        TTreeReaderArray<Float_t>   *LABRQshort{nullptr};
        TTreeReaderArray<Float_t>   *LABRTime{nullptr};
    } labrReader;

  private:
    TRandom3 ranGen;
};
