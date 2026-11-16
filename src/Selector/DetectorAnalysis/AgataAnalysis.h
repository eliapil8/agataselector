#pragma once

#include "Agata.h"
#include "DetectorAnalysis.h"

class AgataAnalysis : public DetectorAnalysis
{
  public:
    explicit AgataAnalysis(Inputs, Agata &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Agata &agata;

    // Histograms
    struct AgataHistograms : public DetectorHistograms
    {
        TH1D *h_trackE{nullptr};
        TH1D *h_AddE{nullptr};
        TH1D *h_coreE0{nullptr};
        TH1D *h_coreE1{nullptr};
        TH1D *h_trackTdiff{nullptr};
        TH1D *h_addTdiff{nullptr};
        TH1D *h_coreTdiff{nullptr};
        TH2D *m_coreE0_coreID{nullptr};
        TH2D *m_coreID_time{nullptr};
        TH2D *m_trackE_trackE{nullptr};
        TH2D *m_AddE_AddE{nullptr};
        TH2D *m_coreE0_coreE0{nullptr};
        TH2D *m_coreE1_coreE1{nullptr};
        struct AgataAnaHistograms : public HistogramStructure
        {
            TH2D *m_layout_XY{nullptr};
            TH2D *m_layout_YZ{nullptr};
            TH2D *m_layout_XZ{nullptr};
            TH3D *m_layout_XYZ{nullptr};
            TH2D *m_EDC_beta{nullptr};
            TH2D *m_CR_E{nullptr};
            TH1D *h_Tdiff{nullptr};
            TH1D *h_E{nullptr};
            TH1D *h_EDC_avg{nullptr};
            TH2D *m_E_E{nullptr};
            TH2D *m_EDC_EDC_avg{nullptr};
            TH2D *m_E_EDC_avg{nullptr};
            TH2D *m_Theta_E_avg{nullptr};
            TH2D *m_Theta_EDC_avg{nullptr};

            struct AgataAntiHistograms : public HistogramStructure
            {
                TH1D *h_CR{nullptr};
                TH2D *m_layout_XY{nullptr};
                TH2D *m_layout_YZ{nullptr};
                TH2D *m_layout_XZ{nullptr};
                TH3D *m_layout_XYZ{nullptr};
                TH2D *m_EDC_beta{nullptr};
                TH2D *m_CR_E{nullptr};
                TH1D *h_E{nullptr};
                TH1D *h_EDC_avg{nullptr};
                TH2D *m_E_E{nullptr};
                TH2D *m_EDC_EDC_avg{nullptr};
                TH2D *m_E_EDC_avg{nullptr};
                TH2D *m_Theta_E_avg{nullptr};
                TH2D *m_Theta_EDC_avg{nullptr};
            } anti;

            struct GammaGatedHistograms : public HistogramStructure
            {
                TH1D               *h_E{nullptr};
                TH1D               *h_EDC_avg{nullptr};
                TH2D               *m_E_E{nullptr};
                TH2D               *m_AddE_AddE{nullptr};
                TH2D               *m_AddE_AddE_mult2{nullptr};
                TH2D               *m_AddE_AddE_mult3{nullptr};
                TH2D               *m_AddE_AddE_mult4{nullptr};
                TH1D               *h_AddE_mult{nullptr};
                TH2D               *m_EDC_EDC_avg{nullptr};
                TH2D               *m_E_EDC_avg{nullptr};
                TH2D               *m_Theta_E_avg{nullptr};
                TH2D               *m_Theta_EDC_avg{nullptr};
                std::pair<int, int> vals{0, 1};
                std::pair<int, int> exclude_angle{80, 100};
                std::string         option;
            };
            std::vector<GammaGatedHistograms> gammagatedHistograms;
        } ana;
    } agataHistograms;

    // Readers
  public:
    struct AgataReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbHits{nullptr};
        TTreeReaderArray<Float_t>   *hitT{nullptr};
        TTreeReaderArray<Float_t>   *hitE{nullptr};
        TTreeReaderArray<Float_t>   *hitX{nullptr};
        TTreeReaderArray<Float_t>   *hitY{nullptr};
        TTreeReaderArray<Float_t>   *hitZ{nullptr};
        TTreeReaderArray<Float_t>   *hitGX{nullptr};
        TTreeReaderArray<Float_t>   *hitGY{nullptr};
        TTreeReaderArray<Float_t>   *hitGZ{nullptr};
        TTreeReaderArray<Int_t>     *hitId{nullptr};
        TTreeReaderArray<Int_t>     *hitSg{nullptr};
        TTreeReaderValue<Int_t>     *nbCores{nullptr};
        TTreeReaderArray<Int_t>     *nbHitsperCry{nullptr};
        TTreeReaderArray<Int_t>     *coreId{nullptr};
        TTreeReaderArray<Float_t>   *coreE0{nullptr};
        TTreeReaderArray<Float_t>   *coreE1{nullptr};
        TTreeReaderArray<Float_t>   *coreT0{nullptr};
        TTreeReaderArray<ULong64_t> *coreTS{nullptr};
        TTreeReaderValue<Int_t>     *nbAdd{nullptr};
        TTreeReaderArray<Int_t>     *AddId{nullptr};
        TTreeReaderArray<Float_t>   *AddE{nullptr};
        TTreeReaderArray<Float_t>   *AddX{nullptr};
        TTreeReaderArray<Float_t>   *AddY{nullptr};
        TTreeReaderArray<Float_t>   *AddZ{nullptr};
        TTreeReaderArray<ULong64_t> *AddTS{nullptr};
        TTreeReaderValue<ULong64_t> *TSHit{nullptr};
        TTreeReaderValue<Int_t>     *nbTrack{nullptr};
        TTreeReaderArray<Float_t>   *trackE{nullptr};
        TTreeReaderArray<Float_t>   *trackX1{nullptr};
        TTreeReaderArray<Float_t>   *trackY1{nullptr};
        TTreeReaderArray<Float_t>   *trackZ1{nullptr};
        TTreeReaderArray<Float_t>   *trackhitE1{nullptr};
        TTreeReaderArray<Float_t>   *trackX2{nullptr};
        TTreeReaderArray<Float_t>   *trackY2{nullptr};
        TTreeReaderArray<Float_t>   *trackZ2{nullptr};
        TTreeReaderArray<Float_t>   *trackhitE2{nullptr};
        TTreeReaderArray<Int_t>     *tracknbHits{nullptr};
        TTreeReaderArray<Int_t>     *trackType{nullptr};
        TTreeReaderArray<Float_t>   *trackFOM{nullptr};
        TTreeReaderArray<Float_t>   *trackT{nullptr};
        TTreeReaderArray<Int_t>     *trackCrystalID{nullptr};
        TTreeReaderValue<ULong64_t> *TStrack{nullptr};
    } agataReader;
};
