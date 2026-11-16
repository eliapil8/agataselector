#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/SpiderAnalysis.h"
#include "GammaDC.h"
#include "Gammadetector.h"
#include "GammadetectorConf.h"
#include "GammadetectorSpiderConf.h"
#include "Spider.h"

class GammadetectorSpiderAnalysis : public CoincidenceAnalysis {
  protected:

  struct GammaParticleHistograms : public CoincidenceHistograms {
    TH1D* h_E{nullptr};
    TH1D* h_EDC{nullptr};
    TH1D* h_EDCBP{nullptr};
    TH2D* m_E_E{nullptr};
    TH2D* m_EDC_EDC{nullptr};
    TH2D* m_EDCBP_EDCBP{nullptr};
    TH2D* m_E_EDC{nullptr};
    TH2D* m_E_EDCBP{nullptr};
    TH2D* m_EDC_EDCBP{nullptr};
    TH2D* m_Beta_E{nullptr};
    TH2D* m_BetaBP_E{nullptr};
    TH2D* m_Beta_EDC{nullptr};
    TH2D* m_BetaBP_EDCBP{nullptr};
    TH2D* m_Theta_E{nullptr};
    TH2D* m_ThetaBP_E{nullptr};
    TH2D* m_Theta_EDC{nullptr};
    TH2D* m_ThetaBP_EDCBP{nullptr};

    bool Fill(Spider&, Gammadetector&, std::vector<GammaDC>&, unsigned int s, unsigned int a1);
    bool Fill2(Spider&, Gammadetector&, std::vector<GammaDC>&, unsigned int s, unsigned int a1, unsigned int a2);
    
  };

  struct GeneralHistograms : public GammaParticleHistograms {
    TH2D* m_Ex_E{nullptr};
    TH2D* m_Ex_EDC{nullptr};
    TH2D* m_Ex_EDCBP{nullptr};
    TH2D* m_SpiderID_EDC{nullptr};
    TH2D* m_SpiderID_EDCBP{nullptr};
  };

  struct GammaGatedHistograms : public GammaParticleHistograms {
    std::pair<int, int> vals{0, 1};
    std::pair<int, int> exclude_angle{80, 100};
    std::string         option;
  };


  struct ExSelection : public GammaParticleHistograms {
    std::pair<double, double> vals{0,0};
    TGraph*                           g_Energy_Theta{nullptr};
    TGraph*                           g_Energy_Theta2{nullptr};
    std::vector<GammaGatedHistograms> gammagatedHistograms;

  };

  struct ETheta : public GammaParticleHistograms {
    TCutG* cut;
    explicit ETheta(TCutG* cut) : cut(cut) {};
    std::vector<GammaGatedHistograms> gammagatedHistograms;
  };

  struct GammadetectorSpiderHistograms : public CoincidenceHistograms {
    GeneralHistograms histos;
    std::vector<GammaGatedHistograms> gammagatedHistograms;
    std::vector<ExSelection> exSelection;
    std::vector<ETheta> eTheta;

  };
  
 protected:
  GammadetectorSpiderAnalysis(Inputs&, GammadetectorSpiderConf&,
			      Gammadetector&, GammadetectorConf&, Spider&,
			      Readers&, SpiderAnalysis::SpiderReader&,
			      std::vector<GammaDC>&,
			      GammadetectorSpiderHistograms&);


  bool AllocateHistos() override;
  void Clear() override;
  void Analyze() override;
  bool FillHistos() override;
  void Finalize() override;

  bool Check(GammaGatedHistograms&, Spider&, Gammadetector&, std::vector<GammaDC>&, unsigned int, unsigned int);
  bool Allocate(GammaParticleHistograms&, std::string);
  
 private:
  Gammadetector&                 gammadetector;
  Spider&                        spider;
  GammadetectorSpiderConf&       gammadetectorSpiderConf;
  GammadetectorConf&             gammadetectorConf;
  std::vector<GammaDC>&          gammaDC;
  GammadetectorSpiderHistograms& gammadetectorSpiderHistograms;
  SpiderAnalysis::SpiderReader&  spiderReader;
  
 private:
  TGraph*      GenerateKinematicLine(const double&);
  unsigned int kineLineCounter{0};
};
