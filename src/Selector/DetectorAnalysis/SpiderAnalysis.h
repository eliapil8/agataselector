#pragma once

#include "DetectorAnalysis.h"
#include "GateSelector.h"
#include "Spider.h"

class SpiderAnalysis : public DetectorAnalysis
{
  public:
    explicit SpiderAnalysis(Inputs, Spider &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Spider &spider;

    // Histograms
    struct SpiderHistograms : public DetectorHistograms
    {
        TH1D *h_mult{nullptr};
        //        TH2D *m_Energy_Time           {nullptr};
        TH2D *m_Energy_TS{nullptr};
        TH2D *m_Theta_Phi{nullptr};
        TH2D *m_layout_XY{nullptr};
        TH2D *m_layout_XZ{nullptr};
        TH2D *m_layout_YZ{nullptr};
        TH2D *m_Theta_E{nullptr};
        TH2D *m_spiderID_E{nullptr};
        TH2D *m_spiderID_E_uncal{nullptr};
        TH1D *h_Ex{nullptr};
        TH2D *m_Theta_Ex{nullptr};
        TH2D *m_Phi_Ex{nullptr};
        TH2D *m_spiderID_Ex{nullptr};
    } spiderHistograms;

    // Readers
  public:
    struct SpiderReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbSPIDER{nullptr};
        TTreeReaderValue<ULong64_t> *TSSPIDER{nullptr};
        TTreeReaderArray<ULong64_t> *TSSPIDER_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *SPIDERChannel{nullptr};
        TTreeReaderArray<UShort_t>  *SPIDERBoard{nullptr};
        TTreeReaderArray<Float_t>   *SPIDEREnergy{nullptr};
        TTreeReaderArray<Float_t>   *SPIDERTime{nullptr};
    } spiderReader;

    GateSelector<1, 1, TGraph> ExTheta;
    GateSelector<1, 1, TGraph> ExPhi;
    GateSelector<1, 1, TGraph> ExID;

  private:
    std::pair<bool, EnergyLoss> spiderDeadlayerELoss{false, {}};
};
