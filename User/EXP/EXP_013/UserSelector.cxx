#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    UserHistograms& hu = userHistograms;

    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    if(conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma) {
        hu.m_IC_gammaGated = new TH2D("m_IC_gammaGated", "m_IC_gammaGated",
                                      1000, 1000, 11000, 1000, 1000, 11000);

        hu.m_EDC_thRot     = new TH2D(
            "m_EDC_thRot",
            "E DC vs delta theta rotation of MCP;dTheta [deg];EDC [keV]",
            (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1150,
            1350);

        hu.m_EDC_phiRot = new TH2D(
            "m_EDC_phiRot",
            "E DC vs delta phi rotation of MCP;dPhi [deg];EDC [keV]",
            (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100, 1150,
            1350);

        hu.m_EDC_agataTh
            = new TH2D("m_EDC_agataTh",
                       "E DC vs delta theta of Agata;dAngle [deg];EDC [keV]",
                       (thMax - thMin) * 2, thMin - 0.005, thMax - 0.005, 100,
                       1150, 1350);
    }

    if(conf.detectorsPresent.Dante && conf.dante.enable_histos) {
        hu.h_TAC_prisma
            = new TH1D("h_TAC_prisma", "h_TAC_prisma", 4000, 1, 40001);
        hu.m_EDC_TAC_prisma = new TH2D("m_EDC_TAC_prisma", "m_EDC_TAC_prisma",
                                       4000, 1, 40001, 2000, 0, 4000);

        hu.m_E_TAC_prisma = new TH2D("m_E_TAC_prisma", "m_E_TAC_prisma", 4000,
                                     1, 40001, 2000, 0, 4000);

        hu.h_EDC_prisma
            = new TH1D("h_EDC_prisma", "h_EDC_prisma", 2000, 0, 4000);
        hu.h_E_prisma = new TH1D("h_E_prisma", "h_E_prisma", 2000, 0, 4000);

        hu.h_TAC_dante
            = new TH1D("h_TAC_dante", "h_TAC_dante", 4000, 1, 40000);
        hu.m_d3x_d3y_tofgated
            = new TH2D("m_d3x_d3y_tofgated", "m_d3x_d3y_tofgated", 2000, 500,
                       40000, 2000, 500, 40000);

        hu.m_d2x_d2y = new TH2D("m_d2x_d2y", "m_d2x_d2y", 2000, 500, 40000,
                                2000, 500, 40000);

        hu.m_d3x_d3y = new TH2D("m_d3x_d3y", "m_d3x_d3y", 2000, 500, 40000,
                                2000, 500, 40000);

        hu.m_d5x_d5y = new TH2D("m_d5x_d5y", "m_d5x_d5y", 2000, 500, 40000,
                                2000, 500, 40000);

/*
				hu.m_gamma=
				new TH1D("m_gamma",
					"m_gamma",
					4000, 500, 4000),


				hu.m_gammaTOF=
				new TH1D("m_gammaTOF",
					"m_gammaTOF",
					4000, 500, 4000),

*/	}

cutZ50 = new TCutG("z50", 11);
cutZ50->SetVarX(" E, with Prisma-Gamma time gate");
cutZ50->SetVarY("total DE_AB ");
cutZ50->SetTitle("Graph");
cutZ50->SetFillStyle(1000);
cutZ50->SetPoint(0, 6573.75, 5328.17);
cutZ50->SetPoint(1, 6933.29, 5379.38);
cutZ50->SetPoint(2, 7565.83, 5362.62);
cutZ50->SetPoint(3, 8018.6, 5324.45);
cutZ50->SetPoint(4, 8361.5, 5336.55);
cutZ50->SetPoint(5, 8155.1, 5422.2);
cutZ50->SetPoint(6, 7522.56, 5481.79);
cutZ50->SetPoint(7, 7009.86, 5465.96);
cutZ50->SetPoint(8, 6543.78, 5419.41);
cutZ50->SetPoint(9, 6460.55, 5368.2);
cutZ50->SetPoint(10, 6573.75, 5328.17);

cutZ48 = new TCutG("z48", 10);
cutZ48->SetVarX(" E, with Prisma-Gamma time gate");
cutZ48->SetVarY("total DE_AB ");
cutZ48->SetTitle("Graph");
cutZ48->SetFillStyle(1000);
cutZ48->SetPoint(0, 6533.8, 5220.17);
cutZ48->SetPoint(1, 6866.71, 5242.52);
cutZ48->SetPoint(2, 7282.86, 5248.1);
cutZ48->SetPoint(3, 7492.59, 5222.97);
cutZ48->SetPoint(4, 7615.77, 5191.31);
cutZ48->SetPoint(5, 7346.11, 5156.86);
cutZ48->SetPoint(6, 6770.17, 5155);
cutZ48->SetPoint(7, 6477.2, 5141.04);
cutZ48->SetPoint(8, 6397.3, 5204.35);
cutZ48->SetPoint(9, 6533.8, 5220.17);

//
//     hu.m_dE_E_gammagated=
//     new TH2D("m_dE_E_gammagated",
//              2000, 0, conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
//
//  hu.h_agataCR = new TH1D("h_agataCR","Count Rate per minute of
//  AGATA;Time[min];Counts per min",10080,0,10080);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    UserHistograms& hu = userHistograms;

    if(conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma) {
        for(unsigned int i{0}; i < agataOutput.P.size(); ++i) {
            TLorentzVector pDC = agataOutput.P[i];
            pDC.Boost(-prismaOutput.beta);

            // if(agataOutput.prisma_coinc[i] /*&&
            // cutZ50->IsInside(**prismaAnalysis->prismaReader.IC_E,**prismaAnalysis->prismaReader.IC_DE_AB)*/)
            //    hu.h_EDC_prisma, pDC.E());
            // if(agataOutput.prisma_coinc[i] /*&&
            // cutZ50->IsInside(**prismaAnalysis->prismaReader.IC_E,**prismaAnalysis->prismaReader.IC_DE_AB)*/)
            //    hu.h_E_prisma, agataOutput.E[i]);

            // if(pDC.E() > 1245 && pDC.E() < 1265 &&
            // agataOutput.prisma_coinc[i])
            //    hu.m_IC_gammaGated, **prismaAnalysis->prismaReader.IC_E,
            //          **prismaAnalysis->prismaReader.IC_DE_AB);
            for(double th = thMin; th < thMax; th += 0.5) {
                TVector3       betaMod;
                TLorentzVector pDCmod = agataOutput.P[i];
                TVector3       ionDir
                    = GetPrismaPos(0., 0., 0., 0., 0., 0., th, 0., 0.);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod.Boost(-betaMod);
                hu.m_EDC_thRot->Fill(th, pDCmod.E());

                ionDir = GetPrismaPos(0., 0., 0., 0., 0., th, 0., 0., 0.);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod = agataOutput.P[i];
                pDCmod.Boost(-betaMod);
                hu.m_EDC_phiRot->Fill(th, pDCmod.E());

                pDCmod = agataOutput.P[i];
                pDCmod.RotateY(th * UNITS::deg);
                pDCmod.Boost(-prismaOutput.beta);
                hu.m_EDC_agataTh->Fill(th, pDCmod.E());
            }
        }
    }

    if(conf.detectorsPresent.Dante && conf.dante.enable_histos) {
        double x2{-1}, y2{-1}, tof25{-1};
        double x3{-1}, y3{-1}, tof3{-1};
        double x5{-1}, y5{-1};
        for(size_t i = 0;
            i < danteAnalysis->danteReader.DANTEEnergy->GetSize(); ++i) {
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 2
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 4) {
                tof3 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 2
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 0) {
                x3 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 2
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 1) {
                y3 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 3
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 4) {
                x2 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 3
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 5) {
                y2 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 3
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 8) {
                x5 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 3
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 9) {
                y5 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
            if(danteAnalysis->danteReader.DANTEBoard->At(i) == 3
               && danteAnalysis->danteReader.DANTEChannel->At(i) == 0) {
                tof25 = danteAnalysis->danteReader.DANTEEnergy->At(i);
            }
        }
        if(tof3 > 0) {
            hu.h_TAC_prisma->Fill(tof3);
            for(unsigned int j{0}; j < agataOutput.P.size(); ++j) {
                TLorentzVector pDC = agataOutput.P[j];
                pDC.Boost(-prismaOutput.beta);
                // if(agataOutput.prisma_coinc[j] /*&&
                // cutZ50->IsInside(**prismaAnalysis->prismaReader.IC_E,**prismaAnalysis->prismaReader.IC_DE_AB)*/){
                //    hu.m_EDC_TAC_prisma, tof3, pDC.E());
                //    hu.m_E_TAC_prisma, tof3, agataOutput.E[j]);
                // }
            }
        }
        if(tof25 > 0) hu.h_TAC_dante->Fill(tof25);
        if(x2 > 0 && y2 > 0) hu.m_d2x_d2y->Fill(x2, y2);
        if(x3 > 0 && y3 > 0) hu.m_d3x_d3y->Fill(x3, y3);
        if(x5 > 0 && y5 > 0) hu.m_d5x_d5y->Fill(x5, y5);
        if(x3 > 0 && y3 > 0 && tof3 > 0) hu.m_d3x_d3y_tofgated->Fill(x3, y3);
    }
    // double timeDiff = 0;
    // if(agataOutput.TS >= timestampRangeAgata.first){
    //     timeDiff = (double)(agataOutput.TS - timestampRangeAgata.first)
    //     /1.e8/60.; for(unsigned int i{0}; i<agataOutput.E.size(); i++){
    //       hu.h_agataCR, timeDiff);
    //     }
    // }
    // else {
    //     std::cout << "Something wrong with time stamps; lower time than
    //     reference found " << std::endl;
    // }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}

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
