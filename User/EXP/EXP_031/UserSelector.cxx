#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    outFile.mkdir("User","User's custom analysis");
    outFile.cd("User");

    m_thetaion_gammaDC = new TH2D("m_thetaion_gammaDC", 
				  "Theta ion vs gammaDC; Theta [deg]; gamma Energy [keV]",
				  1000, 10, 40,
				  3000, 0, 3000);
    // hu.h_agataCR = new TH1D("h_agataCR","Count Rate per minute of
    // AGATA;Time[min];Counts per min",10080,0,10080);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    for(int i{0}; i<gammaAgataPrisma.EDC.size(); ++i){
	if (!gammaAgataPrisma.inCoinc.at(i)) continue;
	TVector3 mcpPos;

    	mcpPos.SetXYZ(((prismaInputOutput.mcp_x) + conf.prisma.xShift)
    	              * conf.prisma.xScale,
    	              ((prismaInputOutput.mcp_y) + conf.prisma.yShift)
    	              * conf.prisma.yScale,
    	              0.);
    	mcpPos.RotateY(conf.prisma.mcp_angle);
    	mcpPos.RotateX(conf.prisma.mcpThetaX);
    	mcpPos.SetZ(conf.prisma.target_mcp_distance + conf.prisma.zShift);

    	mcpPos.RotateZ(conf.prisma.phi);
    	mcpPos.RotateX(conf.prisma.thetaX);
    	mcpPos.RotateY(conf.prisma.thetaY);
    	mcpPos.RotateY(-conf.prisma.angle);

        m_thetaion_gammaDC->Fill(mcpPos.Theta()/UNITS::deg, gammaAgataPrisma.EDC.at(i).E());
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
