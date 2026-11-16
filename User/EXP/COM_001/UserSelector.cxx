#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    UserHistograms& hu = userHistograms;

    /*
    hu.dir             = outFile.mkdir("User", "User's custom analysis");
    hu.dir->cd();

    UserHistograms::IonHistos histos_33S;
    histos_33S.dir = hu.dir->mkdir("Ion_33S");
    histos_33S.dir->cd();
    histos_33S.transition = 840.974 * UNITS::keV;
        histos_33S.h_tkel =
        new TH1D("h_tkel_33S",
                 Form("TKEL value calculation for %f keV line;TKEL sel;Counts",
                      histos_33S.transition / UNITS::keV),
                 400, -100, 100);
        histos_33S.ptrs.push_back(histos_33S.tkel);
    Initialize<TH2D>(histos_33S.m_EDC_crystalID,
                     new TH2D("m_EDC_crystalID_33S",
                              "EDC vs crystal ID;ID;EDC [keV]", 50, -0.5,
                              50 - 0.5, 1000, 0, 2000),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_EDC_xMCP,
                     new TH2D("m_EDC_xMCP_33S",
                              "E DC vs X shift of MCP;X shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 700,
                              1000),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_EDC_yMCP,
                     new TH2D("m_EDC_yMCP_33S",
                              "E DC vs Y shift of MCP;Y shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 700,
                              1000),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_EDC_zMCP,
                     new TH2D("m_EDC_zMCP_33S",
                              "E DC vs Z shift of MCP;Z shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 700,
                              1000),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_EDC_xScale,
                     new TH2D("m_EDC_xScale_33S",
                              "E DC vs X scale of MCP;X scale;EDC [keV]", 50,
                              0.5, 1.5, 400, 700, 1000),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_EDC_yScale,
                     new TH2D("m_EDC_yScale_33S",
                              "E DC vs Y scale of MCP;Y scale;EDC [keV]", 50,
                              0.5, 1.5, 400, 700, 1000),
                     histos_33S.ptrs);
    Initialize<TH2D>(
        histos_33S.m_EDC_phiRot,
        new TH2D("m_EDC_phiRot_33S",
                 "E DC vs delta phi rotation of MCP;dPhi [deg];EDC [keV]",
                 (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 700,
                 1000),
        histos_33S.ptrs);
    Initialize<TH2D>(
        histos_33S.m_EDCCry_thRot,
        new TH2D(
            "m_EDCCry_thRot_33S",
            "E DC vs delta theta rotation of Cry 31;dTheta [deg];EDC [keV]",
            (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 700,
            1000),
        histos_33S.ptrs);
    Initialize<TH2D>(
        histos_33S.m_EDCCry_phiRot,
        new TH2D("m_EDCCry_phiRot_33S",
                 "E DC vs delta phi rotation of Cry 31;dTphi [deg];EDC [keV]",
                 (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 700,
                 1000),
        histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_layout_XY,
                     new TH2D("m_layoutXY", "gamma position XY;X[mm];Y[mm]",
                              800, -400, 400, 800, -400, 400),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_layout_YZ,
                     new TH2D("m_layoutYZ", "gamma position YZ;Y[mm];Z[mm]",
                              800, -400, 400, 800, -400, 400),
                     histos_33S.ptrs);
    Initialize<TH2D>(histos_33S.m_layout_XZ,
                     new TH2D("m_layoutXZ", "gamma position XZ;X[mm];Z[mm]",
                              800, -400, 400, 800, -400, 400),
                     histos_33S.ptrs);

    UserHistograms::IonHistos histos_31P;
    histos_31P.dir = hu.dir->mkdir("Ion_31P");
    histos_31P.dir->cd();
    histos_31P.transition = 1266.1 * UNITS::keV;
    Initialize<TH1D>(
        histos_31P.h_tkel,
        new TH1D("h_tkel_31P",
                 Form("TKEL value calculation for %f keV line;TKEL sel;Counts",
                      histos_31P.transition / UNITS::keV),
                 400, -100, 100),
        histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_crystalID,
                     new TH2D("m_EDC_crystalID_31P",
                              "EDC vs crystal ID;ID;EDC [keV]", 50, -0.5,
                              50 - 0.5, 1000, 0, 2000),
                     histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_xMCP,
                     new TH2D("m_EDC_xMCP_31P",
                              "E DC vs X shift of MCP;X shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 1100,
                              1500),
                     histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_yMCP,
                     new TH2D("m_EDC_yMCP_31P",
                              "E DC vs Y shift of MCP;Y shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 1100,
                              1500),
                     histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_zMCP,
                     new TH2D("m_EDC_zMCP_31P",
                              "E DC vs Z shift of MCP;Z shift [mm];EDC [keV]",
                              mcpZmax - mcpZmin, mcpZmin, mcpZmax, 400, 1100,
                              1500),
                     histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_xScale,
                     new TH2D("m_EDC_xScale_31P",
                              "E DC vs X scale of MCP;X scale;EDC [keV]", 50,
                              0.5, 1.5, 400, 1100, 1500),
                     histos_31P.ptrs);
    Initialize<TH2D>(histos_31P.m_EDC_yScale,
                     new TH2D("m_EDC_yScale_31P",
                              "E DC vs Y scale of MCP;Y scale;EDC [keV]", 50,
                              0.5, 1.5, 400, 1100, 1500),
                     histos_31P.ptrs);
    Initialize<TH2D>(
        histos_31P.m_EDC_phiRot,
        new TH2D("m_EDC_phiRot_31P",
                 "E DC vs delta phi rotation of MCP;dPhi [deg];EDC [keV]",
                 (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 1100,
                 1500),
        histos_31P.ptrs);
    Initialize<TH2D>(
        histos_31P.m_EDCCry_thRot,
        new TH2D(
            "m_EDCCry_thRot_31P",
            "E DC vs delta theta rotation of Cry 31;dTheta [deg];EDC [keV]",
            (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 700,
            1000),
        histos_31P.ptrs);
    Initialize<TH2D>(
        histos_31P.m_EDCCry_phiRot,
        new TH2D("m_EDCCry_phiRot_31P",
                 "E DC vs delta phi rotation of Cry 31;dTphi [deg];EDC [keV]",
                 (thMax - thMin) * 10, thMin - 0.005, thMax - 0.005, 400, 700,
                 1000),
        histos_31P.ptrs);

    hu.ionList[ReactionFragment::Ion(33, 16)] = histos_33S;
    hu.ionList[ReactionFragment::Ion(31, 15)] = histos_31P;
    */
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process
    UserHistograms& hu = userHistograms;
    /*
        UserHistograms::IonHistos& h33S
            = hu.ionList[ReactionFragment::Ion(33, 16)];
        UserHistograms::IonHistos& h31P
            = hu.ionList[ReactionFragment::Ion(31, 15)];

        if(prismaOutput.A == 33 && prismaOutput.Z == 16) { ProcessIon(h33S); }
        if(prismaOutput.A == 31 && prismaOutput.Z == 15) { ProcessIon(h31P); }*/

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    UserHistograms& hu = userHistograms;
    /*
        UserHistograms::IonHistos& h33S
            = hu.ionList[ReactionFragment::Ion(33, 16)];
        UserHistograms::IonHistos& h31P
            = hu.ionList[ReactionFragment::Ion(31, 15)];

        TerminateIon(h33S, 33, 16);
        TerminateIon(h31P, 31, 15);
        */
}

/*
TVector3 UserSelector::GetPrismaPos() {
    TVector3 mcpPos;
    mcpPos.SetMagThetaPhi(250 * UNITS::mm, (**prismaReader.Theta) * UNITS::deg,
                          (**prismaReader.Phi - 90.) * UNITS::deg);
    mcpPos += TVector3(conf.prisma.xShift, conf.prisma.yShift,
                       conf.prisma.zShift);
    mcpPos.SetX(mcpPos.x() * conf.prisma.xScale);
    mcpPos.SetY(mcpPos.y() * conf.prisma.yScale);
    mcpPos.RotateZ(conf.prisma.phi);
    mcpPos.RotateX(conf.prisma.thetaX);
    mcpPos.RotateY(conf.prisma.thetaY);
    return mcpPos;
}

void UserSelector::ProcessIon(UserHistograms::IonHistos hIon) {
    for(unsigned int i{0}; i < agataOutput.P.size(); ++i) {
        TLorentzVector pDC = agataOutput.P[i];

        if(!settings.optimize) {
            TLorentzVector pMod = pDC;

            for(int z = mcpZmin; z < mcpZmax; ++z) {
                TVector3       ionDir, betaMod;
                TLorentzVector pDCmod = pMod;
                ionDir                = GetPrismaPos();
                ionDir.SetX(ionDir.x() + z * UNITS::mm);
                ionDir.RotateY(-conf.prisma.angle);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_xMCP, z, pDCmod.E());

                ionDir = GetPrismaPos();
                ionDir.SetY(ionDir.y() + z * UNITS::mm);
                ionDir.RotateY(-conf.prisma.angle);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod = pMod;
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_yMCP, z, pDCmod.E());

                ionDir = GetPrismaPos();
                ionDir.SetZ(ionDir.z() + z * UNITS::mm);
                ionDir.RotateY(-conf.prisma.angle);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod = pMod;
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_zMCP, z, pDCmod.E());
            }
            for(double s = 0.5; s < 1.5; s += 0.02) {
                TVector3       ionDir, betaMod;
                TLorentzVector pDCmod = pMod;
                ionDir                = GetPrismaPos();
                ionDir.SetX(ionDir.x() * s);
                ionDir.RotateY(-conf.prisma.angle);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_xScale, s, pDCmod.E());

                ionDir = GetPrismaPos();
                ionDir.SetY(ionDir.y() * s);
                ionDir.RotateY(-conf.prisma.angle);
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(), ionDir.Theta(),
                                       ionDir.Phi());
                pDCmod = pMod;
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_yScale, s, pDCmod.E());
            }
            for(double th = thMin; th < thMax; th += 0.1) {
                TVector3       ionDir, betaMod;
                TLorentzVector pDCmod = pMod;
                betaMod.SetMagThetaPhi(prismaOutput.beta.Mag(),
                                       prismaOutput.beta.Theta()
                                           + th * UNITS::deg,
                                       prismaOutput.beta.Phi());
                pDCmod.Boost(-betaMod);
                // Fill(hIon.m_EDC_thRot,th,pDCmod.E());

                betaMod.SetMagThetaPhi(
                    prismaOutput.beta.Mag(), prismaOutput.beta.Theta(),
                    prismaOutput.beta.Phi() + th * UNITS::deg);
                pDCmod = pMod;
                pDCmod.Boost(-betaMod);
                Fill(hIon.m_EDC_phiRot, th, pDCmod.E());
            }

            if(agataOutput.CoreID[i] == myCryID) {
                Fill(hIon.m_layout_XY, agataOutput.Pos.at(i).x() / UNITS::mm,
                     agataOutput.Pos.at(i).y() / UNITS::mm);
                Fill(hIon.m_layout_XZ, agataOutput.Pos.at(i).x() / UNITS::mm,
                     agataOutput.Pos.at(i).z() / UNITS::mm);
                Fill(hIon.m_layout_YZ, agataOutput.Pos.at(i).y() / UNITS::mm,
                     agataOutput.Pos.at(i).z() / UNITS::mm);
            }
        }

        pDC.Boost(-prismaOutput.beta);
        Fill(hIon.m_EDC_crystalID, agataOutput.CoreID[i], pDC.E());

        if(agataOutput.CoreID[i] == myCryID) {
            for(double th = thMin; th < thMax; th += 0.1) {
                TLorentzVector pDCCry_theta = agataOutput.P[i];
                TLorentzVector pDCCry_phi   = agataOutput.P[i];

                // pDCCry_theta.RotateZ(-20 * UNITS::deg);
                // pDCCry_phi.RotateZ(-20 * UNITS::deg);
                // pDCCry_theta.RotateY(-20 * UNITS::deg);
                // pDCCry_phi.RotateY(-20 * UNITS::deg);

                pDCCry_theta.RotateY(th * UNITS::deg);
                pDCCry_phi.RotateZ(th * UNITS::deg);
                pDCCry_theta.Boost(-prismaOutput.beta);
                pDCCry_phi.Boost(-prismaOutput.beta);
                Fill(hIon.m_EDCCry_thRot, th, pDCCry_theta.E());
                Fill(hIon.m_EDCCry_phiRot, th, pDCCry_phi.E());
            }
        }

        if(TMath::Abs(pDC.E() - hIon.transition / UNITS::keV) < 10) {
            Fill(hIon.h_tkel, prismaOutput.TKEL);
        }
    }
}

void UserSelector::TerminateIon(UserHistograms::IonHistos hIon, int A, int Z) {
    hIon.dir->cd();
    if(hIon.m_EDC_crystalID != nullptr) {
        for(int i = 0; i < 40; i++) {
            Initialize<TH1D>(
                hIon.h_EDC_arr[i],
                (TH1D*)hIon.m_EDC_crystalID->ProjectionY(
                    Form("h_EDC_crystal_%i_Z%i_A%i", i, Z, A), i + 1, i + 1),
                hIon.ptrs);
        }
    }
}
*/
