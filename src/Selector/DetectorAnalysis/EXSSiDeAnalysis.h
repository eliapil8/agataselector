#pragma once

#include "DetectorAnalysis.h"
#include "EXSSiDe.h"

class EXSSiDeAnalysis : public DetectorAnalysis
{
  public:
    explicit EXSSiDeAnalysis(Inputs, EXSSiDe &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    EXSSiDe &exsside;

    // Histograms
    struct EXSSiDeHistograms : public DetectorHistograms
    {
        // TH1D* h_mult{nullptr};
        //  TH2D *m_Energy_Time{nullptr};
        TH2D *m_Energy_TS{nullptr};
        TH2D *m_exsside_TSdiff{nullptr};
        TH2D *m_layout_XY{nullptr};
        TH2D *m_layout_XZ{nullptr};
        TH2D *m_layout_YZ{nullptr};
        TH2D *m_Energy_Theta{nullptr};
        TH2D *m_Energy_exssideID{nullptr};
        TH2D *m_Energy_exssideID_uncal{nullptr};
        TH2D *m_exssideBrdCh_exssideBrdCh{nullptr};
        TH2D *m_exssideBrdCh_exssideBrdCh_withcond{nullptr};
        TH1F *h_exxside_EDiff{nullptr};
        struct ReactionHistograms : public HistogramStructure
        {
            TH2D *m_Energy_Theta{nullptr};
        };
        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    } exssideHistograms;

    // Readers
  public:
    struct EXSSiDeReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbEXSSiDe{nullptr};
        TTreeReaderValue<ULong64_t> *TSEXSSiDe{nullptr};
        TTreeReaderArray<ULong64_t> *TSEXSSiDe_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *EXSSiDeChannel{nullptr};
        TTreeReaderArray<UShort_t>  *EXSSiDeBoard{nullptr};
        TTreeReaderArray<Float_t>   *EXSSiDeEnergy{nullptr};
        TTreeReaderArray<Float_t>   *EXSSiDeTime{nullptr};
        TTreeReaderArray<Float_t>   *EXSSiDeCfd{nullptr};
    } exssideReader;

  private:
    std::pair<bool, EnergyLoss> exssideDeadlayerELoss{false, {}};
    std::pair<bool, EnergyLoss> exssideAbsorberELoss{false, {}};
};
