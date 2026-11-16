#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    UserSelector(const std::string& options) : AgataSelector(options){};
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////
    struct UserHistograms {
        /*
          std::vector<TObject*> ptrs;
          TDirectory *dir{nullptr};
          struct IonHistos {
              TH1D*  h_tkel          = nullptr;
              TH2D*  m_EDC_crystalID = nullptr;
              TH2D*  m_EDC_xMCP      = nullptr;
              TH2D*  m_EDC_yMCP      = nullptr;
              TH2D*  m_EDC_zMCP      = nullptr;
              TH2D*  m_EDC_xScale    = nullptr;
              TH2D*  m_EDC_yScale    = nullptr;
              // TH2D* m_EDC_thRot     = nullptr;
              TH2D*  m_EDC_phiRot    = nullptr;
              TH2D*  m_EDCCry_thRot  = nullptr;
              TH2D*  m_EDCCry_phiRot = nullptr;
              TH1D*  h_EDC_arr[40];
              TH2D*  m_layout_XY = nullptr;
              TH2D*  m_layout_XZ = nullptr;
              TH2D*  m_layout_YZ = nullptr;
              double transition{}; // keV
          };
          std::unordered_map<ReactionFragment::Ion, IonHistos,
                             ReactionFragment::Ion::hash_fn>
              ionList;
              */
    } userHistograms;

    // double   mcpZmin{-50.};
    // double   mcpZmax{50.};
    // double   thMin{-20.};
    // double   thMax{20.};
    // int      myCryID{31};

    // TVector3 GetPrismaPos();
    // void     ProcessIon(UserHistograms::IonHistos hIon);
    // void     TerminateIon(UserHistograms::IonHistos hIon, int A, int Z);
};
