#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/OscarAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorOscarConf.h"
#include "Oscar.h"

#include <TCutG.h>

class GammadetectorOscarAnalysis : public CoincidenceAnalysis
{
  protected:
    struct GammadetectorOscarHistograms : public CoincidenceHistograms
    {
        // TH2D* m_TSdiff_TS{nullptr};
        TH1D *h_cTime_gated{nullptr};
        // TH1D* h_TSoscarmTSgammadetector{nullptr};
        TH2D *m_TSoscarmTSgammadetector_oscarID{nullptr};
        TH1D *h_E{nullptr};
        TH1D *h_EDC{nullptr};
        TH1D *h_EDCBP{nullptr};
        TH1D *h_EDC_avg{nullptr};
        TH2D *m_E_E{nullptr};
        TH2D *m_EDC_EDC{nullptr};
        TH2D *m_EDCBP_EDCBP{nullptr};
        TH2D *m_E_EDC{nullptr};
        TH2D *m_E_EDCBP{nullptr};
        TH2D *m_EDC_EDCBP{nullptr};
        TH2D *m_E_Beta{nullptr};
        TH2D *m_E_BetaBP{nullptr};
        TH2D *m_EDC_Beta{nullptr};
        TH2D *m_EDCBP_BetaBP{nullptr};
        TH2D *m_E_Theta{nullptr};
        TH2D *m_E_ThetaBP{nullptr};
        TH2D *m_EOscar_Theta_pg{nullptr};
        TH2D *m_EDCBP_Theta{nullptr};
        TH2D *m_EDC_Theta{nullptr};
        TH2D *m_EDCBP_ThetaBP{nullptr};
        TH2D *m_EDCBP_TS{nullptr}; // Added by MR (be careful!)
        TH2D *m_Ex_E{nullptr};
        TH2D *m_Ex_EDC{nullptr};
        TH2D *m_Ex_EDCBP{nullptr};
        TH2D *m_OscarID_EDC{nullptr};
        TH2D *m_OscarID_EDCBP{nullptr};

        struct ExSelection : public HistogramStructure
        {
            TH1D                     *h_E{nullptr};
            TH1D                     *h_EDC{nullptr};
            TH1D                     *h_EDCBP{nullptr};
            TH2D                     *m_E_Theta{nullptr};
            TH2D                     *m_E_ThetaBP{nullptr};
            TH2D                     *m_EDCBP_Theta{nullptr};
            TH2D                     *m_EDCBP_ThetaBP{nullptr};
            TH2D                     *m_E_E{nullptr};
            TH2D                     *m_EDC_EDC{nullptr};
            TH2D                     *m_EDCBP_EDCBP{nullptr};
            TH2D                     *m_E_EDC{nullptr};
            TH2D                     *m_E_EDCBP{nullptr};
            TH2D                     *m_EDC_EDCBP{nullptr};
            TH2D                     *m_E_Beta{nullptr};
            TH2D                     *m_E_BetaBP{nullptr};
            std::pair<double, double> vals{0, 0};
            TGraph                   *g_Energy_Theta{nullptr};
            TGraph                   *g_Energy_Theta2{nullptr};
            // std::vector<GammaGatedHistograms> gammagatedHistograms;
        };

        std::vector<ExSelection> exSelection;
        std::vector<ExSelection> exSelectionCbC;

        /** Histograms filled only when the OSCAR particle is inside one E-theta cut. */
        struct EThetaSelection : public ExSelection
        {
            TCutG *cut{nullptr};
            TH1F  *h_Ex{nullptr};
            TH2F  *m_EdE{nullptr};
            TH2F  *m_Etot_dE{nullptr};
            explicit EThetaSelection(TCutG *value) : cut(value) {}
        };

        std::vector<EThetaSelection> energyThetaSelections;
        struct ReactionHistograms : public HistogramStructure
        {
            TH1D *h_E{nullptr};
            TH1D *h_EDCBP{nullptr};
            TH1D *h_EDC_avg{nullptr};
            TH2D *m_Ex_E{nullptr};
            TH2D *m_Ex_EDCBP{nullptr};
            TH2D *m_E_ThetaBP{nullptr};
            TH2D *m_E_E{nullptr};
            TH2D *m_OscarTel_Ex{nullptr};
        };
        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionHistograms,
                           ReactionFragment::Ion::vec_hash_fn>
            reactionHistos;
    };

  protected:
    GammadetectorOscarAnalysis(Inputs &,
                               GammadetectorOscarConf &,
                               Gammadetector &,
                               GammadetectorConf &,
                               Oscar &,
                               OscarConf &,
                               Readers &,
                               OscarAnalysis::OscarReader &,
                               std::vector<GammaDC> &,
                               GammadetectorOscarHistograms &);
    bool AllocateHistos() override;
    void Clear() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

    /** Allocate the standard gated histogram set below one E-theta cut directory. */
    void AllocateEnergyThetaHistograms(GammadetectorOscarHistograms::EThetaSelection &selection);

  private:
    Gammadetector                &gammadetector;
    Oscar                        &oscar;
    OscarConf                    &oscarConf;
    GammadetectorOscarConf       &gammadetectorOscarConf;
    GammadetectorConf            &gammadetectorConf;
    std::vector<GammaDC>         &gammaDC;
    GammadetectorOscarHistograms &gammadetectorOscarHistograms;
    OscarAnalysis::OscarReader   &oscarReader;

  protected:
  private:
    TGraph      *GenerateKinematicLine(const double &);
    unsigned int kineLineCounter{0};
};
