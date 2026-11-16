#pragma once

#include "DetectorAnalysis.h"
#include "Oscar.h"

#include <TLeaf.h>

class OscarAnalysis : public DetectorAnalysis
{
  public:
    explicit OscarAnalysis(Inputs, Oscar &, OscarConf &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Oscar           &oscar;
    const OscarConf &oscarConf;

    // Histograms
    struct OscarHistograms : public DetectorHistograms
    {
        TH1F *h_mult{nullptr};
        //        TH2F *m_Energy_Time           {nullptr};
        TH2F *m_Energy_TS{nullptr};
        TH2F *m_Theta_Phi{nullptr};
        TH2F *m_layout_XY{nullptr};
        TH2F *m_layout_XZ{nullptr};
        TH2F *m_layout_YZ{nullptr};
        TH2F *m_Energy_Theta{nullptr};
        TH2F *m_Energy_oscarID{nullptr};
        TH2F *m_Energy_oscarID_uncal{nullptr};
        TH2F *m_oscarBrdCh_oscarBrdCh{nullptr};
        TH2F *m_oscarBrdCh_oscarBrdCh_withcond{nullptr};
        TH1F *h_TimeDiff{nullptr};
        TH1F *h_Ex{nullptr};
        TH2F *m_dE_E{nullptr};

        struct EnedEneHistograms : public DetectorHistograms
        {
            std::unordered_map<unsigned int, TH2F *> m_EdE_telescope;
        };

        EnedEneHistograms EdEHistograms;
        EnedEneHistograms EplusdEHistograms;
        EnedEneHistograms EdEHistograms_raw;

        struct ExHistograms : public DetectorHistograms
        {
            std::unordered_map<unsigned int, TH1F *> h_Ex_telescope;
        } ExHistograms;

        struct dTHistograms : public DetectorHistograms
        {
            std::unordered_map<unsigned int, TH1F *> h_dT_telescope;
        };

        struct timeEvodTHistograms : public DetectorHistograms
        {
            std::unordered_map<unsigned int, TH2F *> h_timeEvodT_telescope;
        } timeEvodTHistograms_tel;

        dTHistograms dTHistograms_tel, dTHistograms_strip, dTHistograms_pad;

        struct ReactionHistograms : public HistogramStructure
        {
            TH1F *h_Ex{nullptr};
            TH2F *m_Energy_Theta{nullptr};
        };
        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    } oscarHistograms;

    // Readers
  public:
    struct OscarReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbOSCAR{nullptr};
        TTreeReaderValue<ULong64_t> *TSOSCAR{nullptr};
        TLeaf                       *TSOSCAR_Hit{nullptr};
        TLeaf                       *OSCARChannel{nullptr};
        TLeaf                       *OSCARBoard{nullptr};
        TLeaf                       *OSCAREnergy{nullptr};
        TLeaf                       *OSCARTime{nullptr};
    } oscarReader;

  private:
    std::pair<bool, EnergyLoss> oscarDeadlayerELoss{false, {}};
    std::pair<bool, EnergyLoss> oscarAbsorberELoss{false, {}};
    TVector3                    oscarNormalDir;

    struct OscarHit
    {
        const OscarLut::Layer *layer;
        double                 e;
        double                 e_raw;
        ULong64_t              time;
        float                  cfd_time;

        OscarHit(const OscarLut::Layer *l) : layer(l) {};
    };
    std::vector<OscarHit> fOscarHits;
    ULong64_t             fFirstTS{0};

    struct OscarEvent
    {
        const OscarHit            *hit_E;
        const OscarHit            *hit_dE;
        const OscarLut::Telescope *telescope;
        ReactionFragment::Ion      ion{0, 0};
        TVector3                   pos;
        double                     time;
        float                      dT;

        OscarEvent(const OscarHit            *hit_E,
                   const OscarHit            *hit_dE,
                   const OscarLut::Telescope *telescope)
            : hit_E(hit_E), hit_dE(hit_dE), telescope(telescope) {};
    };
    std::vector<OscarEvent> fOscarEvents;

  private:
    void VetoHits();
    void BuildEvents();
};

class OscarUpAnalysis : public OscarAnalysis
{
  public:
    explicit OscarUpAnalysis(Inputs, Oscar &);
    bool AllocateHistos() override;
    void Analyze() override;
};

class OscarDownAnalysis : public OscarAnalysis
{
  public:
    explicit OscarDownAnalysis(Inputs, Oscar &);
    bool AllocateHistos() override;
    void Analyze() override;
};
