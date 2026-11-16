#pragma once
#include "AgataSelector.h"
#include "Prisma.h"
#include "Labr.h"
#include "Agata.h"

class UserSelector : public AgataSelector {
  public:
    explicit UserSelector(TFile& oFile) : AgataSelector(oFile){};
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;
    TVector3       GetPrismaPos(double dx = 0., double dy = 0., double dz = 0.,
                                double sx = 0., double sy = 0., double dPhi = 0.,
                                double dTh = 0., double intr_angle = 0.,
                                double intr_thetaX = 0.);

  private:
    // USER variable and histogram definition section

    // unsigned long long timeRef = 0;
    


    struct UserHistograms {
        std::vector<TObject*> ptrs;
        TDirectory *dir{nullptr};
        TH2D* m_QoldWay_EDC    = nullptr;
        TH2D* m_QvsBeta        = nullptr;
        TH2D* m_QvsQval        = nullptr;
        TH2D* m_XFP_Q          = nullptr;
        TH1D* h_tkel_1332      = nullptr;
        TH1D* h_EDC            = nullptr;
        TH2D* m_EDC_crystalID  = nullptr;
        TH2D* m_EDC_xMCP       = nullptr;
        TH2D* m_EDC_yMCP       = nullptr;
        TH2D* m_EDC_zMCP       = nullptr;
        TH2D* m_EDC_xScale     = nullptr;
        TH2D* m_EDC_yScale     = nullptr;
        TH2D* m_EDC_thRot      = nullptr;
        TH2D* m_EDC_phiRot     = nullptr;
        TH2D* m_EDC_intrAngle  = nullptr;
        TH2D* m_EDC_intrThetaX = nullptr;
        TH2D* m_EDC_agataTh    = nullptr;
        TH1D* h_EDC_arr[50];
        TH2D* m_EDCavg_crystalID = nullptr;
        TH2D* m_qvalcomp         = nullptr;
        TH2D* m_thetacomp        = nullptr;
        TH2D* m_phicomp          = nullptr;
        TH2D* m_mcp_XY           = nullptr;
        TH2D* m_mcp_XZ           = nullptr;
        TH2D* m_mcp_YZ           = nullptr;
        
        // added by me
        TH2D* m_E_DEAB_clean	= nullptr;
        TH2D* m_E_DEAB_mult1	= nullptr;
        TH2D* QvalG_Mass_28		= nullptr;
        TH2D* QvalF_Mass_28		= nullptr;
        TH2D* QvalG1_Mass_28	= nullptr;
        //TH2D* m_Elabruncal_TS	= nullptr;
        //TH2D* m_Egamma_Qvalnew	= nullptr;
        TH2D* m_E_XFP_28		= nullptr;
	
	//aggiunto da me
	//TH2D* m_Qval_Mass[20];
	/*
	TH2D* m_Qval_Mass_tot	 = nullptr;
	TH2D* m_Qval_Mass_24	 = nullptr;
	TH2D* m_Qval_Mass_25	 = nullptr;
	TH2D* m_Qval_Mass_26	 = nullptr;
	TH2D* m_Qval_Mass_27	 = nullptr;
	TH2D* m_Qval_Mass_28	 = nullptr;
	TH2D* m_Qval_Mass_29	 = nullptr;
	TH2D* m_Qval_Mass_30	 = nullptr;
	*/
	struct QvalMass {
    		std::vector<TObject*> ptrs;
        	TDirectory *dir{nullptr};
                //std::vector<TH2D*> m_Qval_Mass;
		TH2D* m_Qval_Mass_tot	 = nullptr;
		TH2D* m_Qval_Mass_24	 = nullptr;
		TH2D* m_Qval_Mass_25	 = nullptr;
		TH2D* m_Qval_Mass_26	 = nullptr;
		TH2D* m_Qval_Mass_27	 = nullptr;
		TH2D* m_Qval_Mass_28	 = nullptr;
		TH2D* m_Qval_Mass_29	 = nullptr;
		TH2D* m_Qval_Mass_30	 = nullptr;
            } qvalMass;
    } userHistograms;
    
    // other variables

    double   A1{0};
    double   A2{0};
    double   A3{0};
    double   A4{0};
    double   mcpZmin{-50.};
    double   mcpZmax{50.};
    double   thMin{-50.};
    double   thMax{50.};
    double   theta_ID[50];
    double   phi_ID[50];
    TVector3 betaAvg;
    
    //added by me
    int		 nTOF		= 0;
    double	 Elab		= 0;
    double	 bx		= 0;
    double	 by		= 0;
    double	 bz		= 0;
    double	 QvalF		= 0;
    double	 QvalG		= 0;
    double	 QvalG1		= 0;
    
};
