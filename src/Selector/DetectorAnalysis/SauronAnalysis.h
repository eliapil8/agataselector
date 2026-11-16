#pragma once

#include "DetectorAnalysis.h"
#include "GateSelector.h"
#include "Sauron.h"

class SauronAnalysis : public DetectorAnalysis
{
  protected:
    explicit SauronAnalysis(Inputs, Sauron &, SauronConf &, const std::string &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Sauron           &sauron;
    const SauronConf &sauronConf;

    // Histograms
    struct SauronHistograms : public DetectorHistograms
    {
        TH1D *h_mult{nullptr};
        // TH2D *m_Energy_Time{nullptr};
        TH2D *m_Energy_TS{nullptr};
        TH2D *m_Theta_Phi{nullptr};
        TH2D *m_layout_XY{nullptr};
        TH2D *m_layout_XZ{nullptr};
        TH2D *m_layout_YZ{nullptr};
        TH2D *m_Energy_Theta{nullptr};
        TH2D *m_Energy_sauronID{nullptr};
        TH2D *m_Energy_sauronID_uncal{nullptr};
        TH2D *m_sauronBrdCh_sauronBrdCh{nullptr};
        TH2D *m_sauronBrdCh_sauronBrdCh_withcond{nullptr};
        TH1D *h_Ex{nullptr};
        struct PidHistograms : public HistogramStructure
        {
            std::map<int, TH2D *> m_Imax_Energy{};
        } pidHistograms;
        struct ReactionHistograms : public HistogramStructure
        {
            TH1D *h_Ex{nullptr};
            TH1D *m_Ex_sauronId{nullptr};
            TH2D *m_Ex_Theta{nullptr};
            TH2D *m_Energy_Theta{nullptr};
        };
        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    } sauronHistograms;

    // Readers
  public:
    struct SauronReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbSAURON{nullptr};
        TTreeReaderValue<ULong64_t> *TSSAURON{nullptr};
        TTreeReaderArray<ULong64_t> *TSSAURON_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *SAURONChannel{nullptr};
        TTreeReaderArray<UShort_t>  *SAURONBoard{nullptr};
        TTreeReaderArray<Float_t>   *SAURONEnergy{nullptr};
        TTreeReaderArray<Float_t>   *SAURONTime{nullptr};
        TTreeReaderArray<Float_t>   *SAURONCfd{nullptr};
        TTreeReaderArray<Float_t>   *SAURONImax{nullptr};
        TTreeReaderArray<Float_t>   *SAURONPid{nullptr};
    } sauronReader;

  private:
    std::pair<bool, EnergyLoss> sauronDeadlayerELoss{false, {}};
    std::pair<bool, EnergyLoss> sauronAbsorberELoss{false, {}};
    GateSelector<2, 2>          particleGates;
};

class SauronFwAnalysis : public SauronAnalysis
{
  public:
    explicit SauronFwAnalysis(Inputs, Sauron &);
    bool AllocateHistos();
};

class SauronBwAnalysis : public SauronAnalysis
{
  public:
    explicit SauronBwAnalysis(Inputs, Sauron &);
    bool AllocateHistos() override;
};
