#include "UserSelector.h"
#include "/agata07_data5/andreetta/EXP_011/agataselector/src/Selector/DetectorAnalysis/DetectorAnalysis.h"

#include "/agata07_data5/andreetta/EXP_011/agataselector/src/Selector/DetectorAnalysis/PrismaAnalysis.h"
#include "/agata07_data5/andreetta/EXP_011/agataselector/src/Selector/DetectorAnalysis/AgataAnalysis.h"
#include "/agata07_data5/andreetta/EXP_011/agataselector/src/Selector/DetectorAnalysis/LabrAnalysis.h"

#include "/agata07_data5/andreetta/EXP_011/agataselector/src/Physics/ReactionFragment.h"

#include "Prisma.h"
#include "Labr.h"
#include "Agata.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    //aggiunto da me    
    UserHistograms& hu = userHistograms;
    
    hu.dir = outFile.mkdir("User", "User's custom analysis");
    //outFile.cd("User");
    
    // miscellaneous
    hu.dir->cd();
    hu.m_E_DEAB_clean=
    		new TH2D("m_E_DEAB_clean", "DE_AB : E (no C[0] spike)", 1000, 0, 10000, 1000, 0, 8000);
    hu.m_E_DEAB_mult1=
    		new TH2D("m_E_DEAB_mult1", "DE_AB : E (TOF multiplicity 1)", 1000, 0, 10000, 1000, 0, 8000);
    //hu.m_Elabruncal_TS=
    //		new TH2D("m_Elabruncal_TS", "E_LaBr : PrismaTS", 400, 50, 70, 280, -70, 70);
    //hu.m_Egamma_Qvalnew=
    //		new TH2D("m_Egamma_Qvalnew", "Egamma : Qval", 400, 50, 70, 280, -70, 70);
    hu.m_E_XFP_28=
    		new TH2D("m_E_XFP_28", "E : XFP (Ni)", 1000, 0, 1000, 1000, 0, 10000);
    		
    hu.QvalG_Mass_28=
    		new TH2D("m_QG_Mass_28", "QvalG : Mass (Ni)", 100, 55, 65, 100, -50, 50);
    hu.QvalF_Mass_28=
    		new TH2D("m_QF_Mass_28", "QvalF : Mass (Ni)", 100, 55, 65, 100, -50, 50);
    hu.QvalG1_Mass_28=
    		new TH2D("m_QG1_Mass_28", "QvalG : Mass (Ni)", 100, 55, 65, 100, -50, 50);
    
    //Qval vs Masses
    hu.qvalMass.dir = hu.dir->mkdir("Qval_Mass","Qval vs Mass");
    hu.qvalMass.dir->cd();
    
    //hu.dir = outFile.mkdir("Qval_Mass", "Prisma Q-value:Mass analyzed histograms");
    //hu.dir->cd();

    //hu.qval_mass = hu.dir->mkdir("Qval_Mass", "Prisma Q-value:Mass analyzed histograms");
    //hu.dir->mkdir("Qval_Mass", "Prisma Q-value:Mass analyzed histograms");
    //std::cout << "Qval_Mass folder created" << std::endl;
    //hu.qval_mass.dir->cd();
    
    //QvalMass hu.qval_mass.m_Qval_Mass_tot;
    		      
    hu.qvalMass.m_Qval_Mass_tot=
    		     new TH2D("m_Qval_Mass_tot", "total Q-val : Mass", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_24=
    		     new TH2D("m_Qval_Mass_24", "total Q-val : Mass (Z=24)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_25=
    		     new TH2D("m_Qval_Mass_25", "total Q-val : Mass (Z=25)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_26=
    		     new TH2D("m_Qval_Mass_26", "total Q-val : Mass (Z=26)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_27=
    		     new TH2D("m_Qval_Mass_27", "total Q-val : Mass (Z=27)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_28=
    		     new TH2D("m_Qval_Mass_28", "total Q-val : Mass (Z=28)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_29=
    		     new TH2D("m_Qval_Mass_29", "total Q-val : Mass (Z=29)", 400, 50, 70, 280, -70, 70);
    hu.qvalMass.m_Qval_Mass_30=
    		     new TH2D("m_Qval_Mass_30", "total Q-val : Mass (Z=30)", 400, 50, 70, 280, -70, 70);
	
    /*
    for(int i = 0; i <= conf.prisma.Zrange.second-conf.prisma.Zrange.first; i++) {
    		hu.m_Qval_Mass[i]= 
			new TH2D(Form("m_Qval_Mass_%i", conf.prisma.Zrange.first+i), Form("Qval : Mass (Z = %i)", conf.prisma.Zrange.first+i), 200, 50, 70, 200, -50, 50);
    }
    */
    
/*
    for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++) {
            hu.qval_mass.m_Qval_Mass.push_back(nullptr);
            hu.qval_mass.m_Qval_Mass.at(i)=
                             new TH2D(Form("m_Qval_Mass_%i", i),
                                      Form("Qval : Mass (Z = %i)", i), 200, 50, 70, 200, -50, 50);
    }
    */
    
/*    Initialize<TH2D>(hu.qval_mass.m_Qval_Mass_tot,
                         new TH2D("m_Qval_Mass_tot", "total Q-val : Mass",
                                  200, 50, 70, 200, -50, 50),
                         hu.qval_mass.ptrs);
    
    for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++) {
            hu.qval_mass.m_Qval_Mass.push_back(nullptr);
            Initialize<TH2D>(hu.qval_mass.m_Qval_Mass.back(),
                             new TH2D(Form("m_Qval_Mass_%i", i),
                                      Form("Qval : Mass (Z = %i)", i), 200, 50, 70, 200, -50, 50),
                             hu.qval_mass.ptrs);
    }
*/

/*
    hu.h_EDC           = new TH1D("h_EDC", "E gamma DC total", 6000, 0, 6000);

    hu.m_QvsBeta, new TH2D("m_QvsBeta", "m_QvsBeta;Q [-];Beta [#%]", 50, 0, 50,
                           400, 0, 20);

    hu.m_QvsQval, new TH2D("m_QvsQval", "m_QvsQval;Q [-];Q-val [MeV]", 50, 0,
                           50, 200, -50, 50);

    hu.m_XFP_Q,
        new TH2D("m_XFP_Q", "m_XFP_Q;XFP ;Q", 1000, 0, 1000, 50, 0, 50);

    hu.m_QoldWay_EDC, new TH2D("m_QoldWay_EDC",
                               "Q value with non relativistic "
                               "formula vs EDC;Q-val [MeV];EDC [keV]",
                               200, -100, 100, 3000, 0, 6000);

    hu.m_qvalcomp, new TH2D("m_qvalcomp",
                            "Q value calculation comparison"
                            "histo;TKEL sel;Q-val prisma",
                            200, -100, 100, 200, -100, 100);

    hu.h_tkel_1332, new TH1D("h_tkel_1332",
                             "TKEL value calculation for 1332 keV line"
                             ";TKEL sel;Counts",
                             200, -100, 100);

    hu.m_EDC_crystalID,
        new TH2D("m_EDC_crystalID", "EDC vs crystal ID;ID;EDC [keV]", 50, -0.5,
                 50 - 0.5, 6000, 0, 6000);

    hu.m_EDCavg_crystalID,
        new TH2D("m_EDCavg_crystalID",
                 "EDC with beta avg vs crystal ID;ID;EDC [keV]", 50, -0.5,
                 50 - 0.5, 6000, 0, 6000);

    hu.m_EDC_xMCP,
        new TH2D("m_EDC_xMCP", "E DC vs X shift of MCP;X shift [mm];EDC [keV]",
                 mcpZmax - mcpZmin, mcpZmin, mcpZmax, 100, 1300, 1400);

    hu.m_EDC_yMCP,
        new TH2D("m_EDC_yMCP", "E DC vs Y shift of MCP;Y shift [mm];EDC [keV]",
                 mcpZmax - mcpZmin, mcpZmin, mcpZmax, 100, 1300, 1400);

    hu.m_EDC_zMCP,
        new TH2D("m_EDC_zMCP", "E DC vs Z shift of MCP;Z shift [mm];EDC [keV]",
                 mcpZmax - mcpZmin, mcpZmin, mcpZmax, 100, 1300, 1400);

    hu.m_EDC_xScale
        = new TH2D("m_EDC_xScale", "E DC vs X scale of MCP;X scale;EDC [keV]",
                   50, -0.5, 0.5, 100, 1300, 1400);

    hu.m_EDC_yScale
        = new TH2D("m_EDC_yScale", "E DC vs Y scale of MCP;Y scale;EDC [keV]",
                   50, -0.5, 0.5, 100, 1300, 1400);

    hu.m_EDC_thRot = new TH2D(
        "m_EDC_thRot",
        "E DC vs delta theta rotation of MCP;dTheta [deg];EDC [keV]",
        (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1300, 1400);

    hu.m_EDC_phiRot = new TH2D(
        "m_EDC_phiRot",
        "E DC vs delta phi rotation of MCP;dPhi [deg];EDC [keV]",
        (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1300, 1400);

    hu.m_EDC_intrAngle = new TH2D("m_EDC_intrAngle",
                                  "E DC vs delta intrinsic theta rotation of "
                                  "MCP;dAngle [deg];EDC [keV]",
                                  (thMax - thMin) * 2, thMin - 0.005,
                                  thMax - 0.005, 100, 1300, 1400);

    hu.m_EDC_agataTh   = new TH2D(
        "m_EDC_agataTh", "E DC vs delta theta of Agata;dAngle [deg];EDC [keV]",
        (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1300, 1400);

    hu.m_EDC_intrThetaX = new TH2D(
        "m_EDC_intrThetaX",
        "E DC vs delta intrinsic rotation around X axis "
        "of MCP;dAngle [deg];EDC [keV]",
        (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1300, 1400);

    hu.m_thetacomp = new TH2D(
        "m_thetacomp", "Theta prisma vs selector;Theta prisma;Theta sel", 100,
        -50, 50, 100, -50, 50);

    hu.m_phicomp
        = new TH2D("m_phicomp", "Phi prisma vs selector;Phi prisma;Phi sel",
                   360, 0, 360, 360, -180, 180);

    hu.m_mcp_XY = new TH2D("m_mcp_XY", "MCP layout XY;X [mm];Y [mm]", 800,
                           -400, 400, 800, -400, 400);
    hu.m_mcp_XZ = new TH2D("m_mcp_XZ", "MCP layout XZ;X [mm];Z [mm]", 800,
                           -400, 400, 800, -400, 400);
    hu.m_mcp_YZ = new TH2D("m_mcp_YZ", "MCP layout YZ;Y [mm];Z [mm]", 800,
                           -400, 400, 800, -400, 400);

    ReactionFragment f1{
        {116, 50},
        "./Conf/NuclearData/nutab12"
    };
    ReactionFragment f2{
        {60, 28},
        "./Conf/NuclearData/nutab12"
    };

    // A1 = A3 = f1.Get_M();
    // A2 = A4 = f2.Get_M();
    A2 = A3 = f1.Get_M();
    A1 = A4 = f2.Get_M();
*/

    betaAvg.SetMagThetaPhi(0.11, 20 * UNITS::deg, 180);

    theta_ID[0]  = conf.user.ATC00_theta;
    theta_ID[1]  = conf.user.ATC01_theta;
    theta_ID[2]  = conf.user.ATC02_theta;
    theta_ID[3]  = conf.user.ATC03_theta;
    theta_ID[4]  = conf.user.ATC04_theta;
    theta_ID[5]  = conf.user.ATC05_theta;
    theta_ID[6]  = conf.user.ATC06_theta;
    theta_ID[7]  = conf.user.ATC07_theta;
    theta_ID[8]  = conf.user.ATC08_theta;
    theta_ID[9]  = conf.user.ATC09_theta;
    theta_ID[10] = conf.user.ATC10_theta;
    theta_ID[11] = conf.user.ATC11_theta;
    theta_ID[12] = conf.user.ATC12_theta;
    theta_ID[13] = conf.user.ATC13_theta;
    theta_ID[14] = conf.user.ATC14_theta;
    theta_ID[15] = conf.user.ATC15_theta;
    theta_ID[16] = conf.user.ATC16_theta;
    theta_ID[17] = conf.user.ATC17_theta;
    theta_ID[18] = conf.user.ATC18_theta;
    theta_ID[19] = conf.user.ATC19_theta;
    theta_ID[20] = conf.user.ATC20_theta;
    theta_ID[21] = conf.user.ATC21_theta;
    theta_ID[22] = conf.user.ATC22_theta;
    theta_ID[23] = conf.user.ATC23_theta;
    theta_ID[24] = conf.user.ATC24_theta;
    theta_ID[25] = conf.user.ATC25_theta;
    theta_ID[26] = conf.user.ATC26_theta;
    theta_ID[27] = conf.user.ATC27_theta;
    theta_ID[28] = conf.user.ATC28_theta;
    theta_ID[29] = conf.user.ATC29_theta;
    theta_ID[30] = conf.user.ATC30_theta;
    theta_ID[31] = conf.user.ATC31_theta;
    theta_ID[32] = conf.user.ATC32_theta;
    theta_ID[33] = conf.user.ATC33_theta;
    theta_ID[34] = conf.user.ATC34_theta;
    theta_ID[35] = conf.user.ATC35_theta;
    theta_ID[36] = conf.user.ATC36_theta;
    theta_ID[37] = conf.user.ATC37_theta;
    theta_ID[38] = conf.user.ATC38_theta;
    theta_ID[39] = conf.user.ATC39_theta;
    theta_ID[40] = conf.user.ATC40_theta;
    theta_ID[41] = conf.user.ATC41_theta;
    theta_ID[42] = conf.user.ATC42_theta;

    phi_ID[0]    = conf.user.ATC00_phi;
    phi_ID[1]    = conf.user.ATC01_phi;
    phi_ID[2]    = conf.user.ATC02_phi;
    phi_ID[3]    = conf.user.ATC03_phi;
    phi_ID[4]    = conf.user.ATC04_phi;
    phi_ID[5]    = conf.user.ATC05_phi;
    phi_ID[6]    = conf.user.ATC06_phi;
    phi_ID[7]    = conf.user.ATC07_phi;
    phi_ID[8]    = conf.user.ATC08_phi;
    phi_ID[9]    = conf.user.ATC09_phi;
    phi_ID[10]   = conf.user.ATC10_phi;
    phi_ID[11]   = conf.user.ATC11_phi;
    phi_ID[12]   = conf.user.ATC12_phi;
    phi_ID[13]   = conf.user.ATC13_phi;
    phi_ID[14]   = conf.user.ATC14_phi;
    phi_ID[15]   = conf.user.ATC15_phi;
    phi_ID[16]   = conf.user.ATC16_phi;
    phi_ID[17]   = conf.user.ATC17_phi;
    phi_ID[18]   = conf.user.ATC18_phi;
    phi_ID[19]   = conf.user.ATC19_phi;
    phi_ID[20]   = conf.user.ATC20_phi;
    phi_ID[21]   = conf.user.ATC21_phi;
    phi_ID[22]   = conf.user.ATC22_phi;
    phi_ID[23]   = conf.user.ATC23_phi;
    phi_ID[24]   = conf.user.ATC24_phi;
    phi_ID[25]   = conf.user.ATC25_phi;
    phi_ID[26]   = conf.user.ATC26_phi;
    phi_ID[27]   = conf.user.ATC27_phi;
    phi_ID[28]   = conf.user.ATC28_phi;
    phi_ID[29]   = conf.user.ATC29_phi;
    phi_ID[30]   = conf.user.ATC30_phi;
    phi_ID[31]   = conf.user.ATC31_phi;
    phi_ID[32]   = conf.user.ATC32_phi;
    phi_ID[33]   = conf.user.ATC33_phi;
    phi_ID[34]   = conf.user.ATC34_phi;
    phi_ID[35]   = conf.user.ATC35_phi;
    phi_ID[36]   = conf.user.ATC36_phi;
    phi_ID[37]   = conf.user.ATC37_phi;
    phi_ID[38]   = conf.user.ATC38_phi;
    phi_ID[39]   = conf.user.ATC39_phi;
    phi_ID[40]   = conf.user.ATC40_phi;
    phi_ID[41]   = conf.user.ATC41_phi;
    phi_ID[42]   = conf.user.ATC42_phi;
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    UserHistograms& hu = userHistograms;
      
    if(**prismaAnalysis->prismaReader.Traj_OK && ((*(prismaAnalysis->prismaReader.IC_C_raw)).At(0)<260 || prismaAnalysis->prismaReader.IC_C_raw->At(0)>300) && (**prismaAnalysis->prismaReader.IC_E>1.4152942*( **prismaAnalysis->prismaReader.RBeta)-43.910308)) hu.m_E_DEAB_clean->Fill( **prismaAnalysis->prismaReader.IC_E, **prismaAnalysis->prismaReader.IC_DE_AB);
    //hu.m_Elabruncal_TS->Fill( **prismaAnalysis->prismaReader.TSPrisma, *labrAnalysis->labrReader.LABREnergy);
    //if(**prismaAnalysis->prismaReader.Z_Nr==28&&**prismaAnalysis->prismaReader.A_Nr==60) hu.m_Egamma_Qvalnew->Fill( **prismaAnalysis->prismaReader.Qvalue, **agataAnalysis->agataReader.coreE1);
    if(**prismaAnalysis->prismaReader.Traj_OK && **prismaAnalysis->prismaReader.Z_Nr == 28) hu.m_E_XFP_28->Fill( **prismaAnalysis->prismaReader.X_FP, **prismaAnalysis->prismaReader.IC_E);
   
	// velocity components
	bz = (TMath::Sqrt(1+4*Beta*Beta*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Theta*TMath::Pi()/180)*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Theta*TMath::Pi()/180)*(1+TMath::Tan(**prismaAnalysis->prismaReader.MCP_Phi*TMath::Pi()/180)*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Phi*TMath::Pi()/180))) - 1)/(2*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Theta*TMath::Pi()/180)*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Theta*TMath::Pi()/180)*(1+TMath::Tan(**prismaAnalysis->prismaReader.MCP_Phi*TMath::Pi()/180)*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Phi*TMath::Pi()/180)));
	by = bz*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Theta*TMath::Pi()/180);
	bx = by*TMath::Tan(**prismaAnalysis->prismaReader.MCP_Phi*TMath::Pi()/180);

    Elab = 460;
    QvalG = 176./(176-(**prismaAnalysis->prismaReader.Mass))*0.5*(**prismaAnalysis->prismaReader.Mass)*937*(bx*bx+by*by+(bz-TMath::Sqrt(2*(Elab-8.741)*116/(176*176*937)))*(bz-TMath::Sqrt(2*(Elab-8.741)*116/(176*176*937)))) - (Elab-8.741)*60/176;
    //QvalG = 176/(176-(**prismaAnalysis->prismaReader.Mass))*0.5*(**prismaAnalysis->prismaReader.Mass)*937*((TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*(TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180) + (TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*(TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180) + ((TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180) - TMath::Sqrt(2*(460-8.741)*116/(176*176*937)))*((TMath::Sqrt(2*((**prismaAnalysis->prismaReader.IC_E)*0.092005131-96.751221)/((**prismaAnalysis->prismaReader.Mass)*937))+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180)-TMath::Sqrt(2*(460-8.741)*116/(176*176*937))))-(Elab-8.741)*60/176;
    
    if(**prismaAnalysis->prismaReader.Z_Nr == 28) hu.QvalG_Mass_28->Fill( **prismaAnalysis->prismaReader.Mass, QvalG);
    
   /QvalF = 176.*937./(2*(**prismaAnalysis->prismaReader.Mass)*(176-(**prismaAnalysis->prismaReader.Mass)))*(**prismaAnalysis->prismaReader.Mass)*(**prismaAnalysis->prismaReader.Mass)*(**prismaAnalysis->prismaReader.Beta)*(**prismaAnalysis->prismaReader.Beta) - ((176-(**prismaAnalysis->prismaReader.Mass))-116)*Elab/(176-(**prismaAnalysis->prismaReader.Mass)) - TMath::Sqrt(2*Elab*116*937)/(176-(**prismaAnalysis->prismaReader.Mass))*(**prismaAnalysis->prismaReader.Mass)*(**prismaAnalysis->prismaReader.Beta)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180);
    
    if(**prismaAnalysis->prismaReader.Z_Nr == 28) hu.QvalF_Mass_28->Fill( **prismaAnalysis->prismaReader.Mass, QvalF);
    
   /QvalG1 = 176/(176-(**prismaAnalysis->prismaReader.Mass))*0.5*(**prismaAnalysis->prismaReader.Mass)*937*(((**prismaAnalysis->prismaReader.Beta)+0.0006)*((**prismaAnalysis->prismaReader.Beta)+0.0006)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180) + ((**prismaAnalysis->prismaReader.Beta)+0.0006)*((**prismaAnalysis->prismaReader.Beta)+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180)*TMath::Sin((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180) + (((**prismaAnalysis->prismaReader.Beta)+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180) - TMath::Sqrt(2*(Elab-8.741)*116/(176*176*937)))*(((**prismaAnalysis->prismaReader.Beta)+0.0006)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Phi)*TMath::Pi()/180)*TMath::Cos((**prismaAnalysis->prismaReader.MCP_Theta)*TMath::Pi()/180)-TMath::Sqrt(2*(Elab-8.741)*116/(176*176*937))))-(Elab-8.741)*60/176;
       
    if(**prismaAnalysis->prismaReader.Z_Nr == 28) hu.QvalG1_Mass_28->Fill( **prismaAnalysis->prismaReader.Mass, QvalG1);
    
    //aggiunto da me
    hu.qvalMass.dir->cd();
    
    hu.qvalMass.m_Qval_Mass_tot->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    
    if(**prismaAnalysis->prismaReader.Z_Nr == 24) hu.qvalMass.m_Qval_Mass_24->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 25) hu.qvalMass.m_Qval_Mass_25->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 26) hu.qvalMass.m_Qval_Mass_26->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 27) hu.qvalMass.m_Qval_Mass_27->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 28) hu.qvalMass.m_Qval_Mass_28->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 29) hu.qvalMass.m_Qval_Mass_29->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    if(**prismaAnalysis->prismaReader.Z_Nr == 30) hu.qvalMass.m_Qval_Mass_30->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    
    
    for (int i=0; i<10; i++) {if(prismaAnalysis->prismaReader.TOF_raw->At(i)>300) nTOF++;}
    if(**prismaAnalysis->prismaReader.Traj_OK && ((*(prismaAnalysis->prismaReader.IC_C_raw)).At(0)<260 || prismaAnalysis->prismaReader.IC_C_raw->At(0)>300) && nTOF==1) hu.m_E_DEAB_mult1->Fill( **prismaAnalysis->prismaReader.IC_E, **prismaAnalysis->prismaReader.IC_DE_AB);
    
    /*
    for(int i = 24; i < 31; i++) {
            if(**prismaAnalysis->prismaReader.Z_Nr == i) hu.m_Qval_Mass[i]->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    }
    */
    //std::cout << "m_Qval_Mass_tot filled" << std::endl;
    /*
    for(unsigned int i = 0; i < hu.qval_mass.m_Qval_Mass.size(); i++) {
            if(**prismaAnalysis->prismaReader.Z_Nr == i + conf.prisma.Zrange.first) {
                if(!hu.qval_mass.m_Qval_Mass.empty()) hu.qval_mass.m_Qval_Mass.at(i)->Fill( **prismaAnalysis->prismaReader.Mass, **prismaAnalysis->prismaReader.Qvalue);
    	    }
    }*/
    
/*        Fill(hu.qval_mass.m_Qval_Mass_tot, **prismaReader.Mass, **prismaReader.Qvalue);
        for(unsigned int i = 0; i < hu.qval_mass.m_Qval_Mass.size(); i++) {
            if(**prismaReader.Z_Nr == i + conf.prisma.Zrange.first) {
                if(!hu.qval_mass.m_Qval_Mass.empty())
                    Fill(hu.qval_mass.m_Qval_Mass.at(i), **prismaReader.Mass, **prismaReader.Qvalue);
            }
        }*/

/*
    if(prismaOutput.A == 60 && prismaOutput.Z == 28) {
        hu.m_QvsBeta->Fill(**prismaAnalysis->prismaReader.q_Nr,
                           prismaOutput.beta.Mag() * 100);
        hu.m_XFP_Q->Fill(**prismaAnalysis->prismaReader.X_FP,
                         **prismaAnalysis->prismaReader.q_Nr);

        double d3 = sqrt(
            (A1 * A3 * conf.reaction.energy)
            / (pow(A1 + A2, 2) * (prismaOutput.p3.E() - prismaOutput.p3.M())));
        auto q = (A1 * A3 * conf.reaction.energy / UNITS::MeV)
               / ((A1 + A2) * A4)
               * ((1 + d3 * d3 - 2 * d3 * cos(prismaOutput.p3.Theta()))
                      / (d3 * d3)
                  - (A2 * A4) / (A1 * A3));
        // std::cout << "q: "<<q << std::endl;
        // std::cout << "d3: "<<d3 << std::endl;
        // std::cout << "A1: "<<A1 << std::endl;
        // std::cout << "A2: "<<A2 << std::endl;

        ReactionFragment::Ion ion{prismaOutput.A, prismaOutput.Z};
        ReactionFragment::Ion ionBP
            = conf.reaction.target + conf.reaction.beam - ion;

        // TLorentzVector pIon3
        //     = SlowInTarget(prismaOutput.p3, conf.agataPrisma.dcPos, ion);
        // TLorentzVector pIon4
        //     = SlowInTarget(prismaOutput.p4, conf.agataPrisma.bpDcPos, ionBP);
        TLorentzVector pIon3 = prismaOutput.p3;
        TLorentzVector pIon4 = prismaOutput.p4;

        hu.m_qvalcomp->Fill(prismaOutput.TKEL, q);
        hu.m_QvsQval->Fill(**prismaAnalysis->prismaReader.q_Nr,
                           prismaOutput.TKEL);

        for(unsigned int i{0}; i < agataOutput.P.size(); ++i) {
            TLorentzVector pMod = agataOutput.P[i];
            pMod.RotateY(theta_ID[agataOutput.CoreID[i]]);
            pMod.RotateZ(phi_ID[agataOutput.CoreID[i]]);

            TLorentzVector pDC    = pMod;
            TLorentzVector pDCavg = pMod;
            if(!settings.optimize) {
                for(int z = mcpZmin; z < mcpZmax; ++z) {
                    TVector3       betaMod;
                    TLorentzVector pDCmod = pMod;
                    TVector3       ionDir
                        = GetPrismaPos(z, 0., 0., 0., 0., 0., 0., 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_xMCP->Fill(z, pDCmod.E());

                    ionDir = GetPrismaPos(0., z, 0., 0., 0., 0., 0., 0., 0.);

                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_yMCP->Fill(z, pDCmod.E());

                    ionDir = GetPrismaPos(0., 0., z, 0., 0., 0., 0., 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_zMCP->Fill(z, pDCmod.E());
                }
                for(double s = -0.5; s < 0.5; s += 0.02) {
                    TVector3       betaMod;
                    TLorentzVector pDCmod = pMod;
                    TVector3       ionDir
                        = GetPrismaPos(0., 0., 0., s, 0., 0., 0., 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_xScale->Fill(s, pDCmod.E());

                    ionDir = GetPrismaPos(0., 0., 0., 0., s, 0., 0., 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_yScale->Fill(s, pDCmod.E());
                }
                for(double th = thMin; th < thMax; th += 0.5) {
                    TVector3       betaMod;
                    TLorentzVector pDCmod = pMod;
                    TVector3       ionDir
                        = GetPrismaPos(0., 0., 0., 0., 0., 0., th, 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_thRot->Fill(th, pDCmod.E());

                    ionDir = GetPrismaPos(0., 0., 0., 0., 0., th, 0., 0., 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_phiRot->Fill(th, pDCmod.E());

                    ionDir = GetPrismaPos(0., 0., 0., 0., 0., 0., 0., th, 0.);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_intrAngle->Fill(th, pDCmod.E());

                    ionDir = GetPrismaPos(0., 0., 0., 0., 0., 0., 0., 0., th);
                    betaMod.SetMagThetaPhi(pIon3.Beta(), ionDir.Theta(),
                                           ionDir.Phi());
                    pDCmod = pMod;
                    pDCmod.Boost(-betaMod);
                    hu.m_EDC_intrThetaX->Fill(th, pDCmod.E());

                    pDCmod = pMod;
                    pDCmod.RotateY(th * UNITS::deg);
                    pDCmod.Boost(-prismaOutput.beta);
                    hu.m_EDC_agataTh->Fill(th, pDCmod.E());
                }
            }
            pDC.Boost(-pIon3.BoostVector());
            pDCavg.Boost(-betaAvg);
            hu.m_QoldWay_EDC->Fill(q, pDC.E());
            hu.m_EDC_crystalID->Fill(agataOutput.CoreID[i], pDC.E());
            hu.h_EDC->Fill(pDC.E());
            hu.m_EDCavg_crystalID->Fill(agataOutput.CoreID[i], pDCavg.E());
            if(TMath::Abs(pDC.E() - 1332) < 10) {
                hu.h_tkel_1332->Fill(prismaOutput.TKEL);
            }
        }
    }
    if(prismaOutput.beta.Mag() > 0) {
        hu.m_thetacomp->Fill(**prismaAnalysis->prismaReader.Theta,
                             prismaOutput.beta.Theta() / UNITS::deg);
        hu.m_phicomp->Fill(**prismaAnalysis->prismaReader.Phi,
                           prismaOutput.beta.Phi() / UNITS::deg);

        TVector3 mcpPos;
        mcpPos = GetPrismaPos();

        hu.m_mcp_XY->Fill(mcpPos.x() / UNITS::mm, mcpPos.y() / UNITS::mm);
        hu.m_mcp_XZ->Fill(mcpPos.x() / UNITS::mm, mcpPos.z() / UNITS::mm);
        hu.m_mcp_YZ->Fill(mcpPos.y() / UNITS::mm, mcpPos.z() / UNITS::mm);
    }
    */
    return kTRUE;
    
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    /*
    if(userHistograms.m_EDC_crystalID != nullptr) {
        for(int i = 0; i < 50; i++) {
            userHistograms.h_EDC_arr[i]
                = (TH1D*)userHistograms.m_EDC_crystalID->ProjectionY(
                    ("h_EDC_crystal_" + std::to_string(i)).c_str(), i + 1,
                    i + 1);
        }
    }
    */
    // User terminate
}

/*
TVector3 UserSelector::GetPrismaPos(double dx, double dy, double dz, double sx,
                                    double sy, double dPhi, double dTh,
                                    double intr_angle, double intr_thetaX) {
    TVector3 mcpPos;
    // mcpPos.SetMagThetaPhi(250 * UNITS::mm,
    // (**prismaAnalysis->prismaReader.Theta) * UNITS::deg,
    //                       (**prismaAnalysis->prismaReader.Phi - 90.) *
    //                       UNITS::deg);
    // mcpPos += TVector3(conf.prisma.xShift, conf.prisma.yShift,
    //                    conf.prisma.zShift);
    // mcpPos.SetX(mcpPos.x() * conf.prisma.xScale);
    // mcpPos.SetY(mcpPos.y() * conf.prisma.yScale);

    double   mcp_distance   = 250 * UNITS::mm;
    double   mcp_intr_angle = 135 * UNITS::deg;

    mcpPos.SetXYZ(((**prismaAnalysis->prismaReader.MCP_X) * UNITS::mm
                   + conf.prisma.xShift + dx * UNITS::mm)
                      * (conf.prisma.xScale + sx),
                  ((**prismaAnalysis->prismaReader.MCP_Y) * UNITS::mm
                   + conf.prisma.yShift + dy * UNITS::mm)
                      * (conf.prisma.yScale + sy),
                  0.);
    mcpPos.RotateY(mcp_intr_angle
                   - (conf.prisma.mcpAngle + intr_angle * UNITS::deg));
    mcpPos.RotateX(conf.prisma.mcpThetaX + intr_thetaX * UNITS::deg);
    mcpPos.SetZ(mcp_distance + conf.prisma.zShift + dz * UNITS::mm);

    mcpPos.RotateZ(conf.prisma.phi + dPhi * UNITS::deg);
    mcpPos.RotateX(conf.prisma.thetaX);

    mcpPos.RotateY(
        -(conf.prisma.angle + conf.prisma.thetaY + dTh * UNITS::deg));

    return mcpPos;
}
*/
