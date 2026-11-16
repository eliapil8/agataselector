#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& h = userHistograms;

    h.h_coreE0        = new TH1D("h_coreE0", "h_coreE0", 4000, 0, 4000);
    h.h_AddE          = new TH1D("h_AddE", "h_AddE", 4000, 0, 4000);
    h.h_trackE        = new TH1D("h_trackE", "h_trackE", 4000, 0, 4000);
    h.m_coreE0_coreE0 = new TH2D("m_coreE0_coreE0", "m_coreE0_coreE0", 4000, 0,
                                 4000, 4000, 0, 4000);
    h.m_AddE_AddE
        = new TH2D("m_AddE_AddE", "m_AddE_AddE", 4000, 0, 4000, 4000, 0, 4000);
    h.m_trackE_trackE = new TH2D("m_trackE_trackE", "m_trackE_trackE", 4000, 0,
                                 4000, 4000, 0, 4000);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    double lowestT{std::numeric_limits<double>::max()};
    double lowestcoreTS{std::numeric_limits<double>::max()};
    int    lowestIdx{0};
    int    lowestcoreIdx{0};
    // Fill Agata tree

    auto   ana
        = [&](ULong64_t& TS, std::vector<Float_t>& e, std::vector<Float_t>& t,
              std::vector<Float_t>& x, std::vector<Float_t>& y,
              std::vector<Float_t>& z, std::vector<Int_t>& core) {
              agataOutput.clear();
              lowestT   = std::numeric_limits<double>::max();
              lowestIdx = 0;
              for(size_t i{0}; i < e.size(); ++i) {
                  if(lowestT > t.at(i)) {
                      lowestT   = t.at(i);
                      lowestIdx = i;
                  }
              }
              agataOutput.TS = TS;
              std::vector<unsigned int> idx;
              for(size_t i{0}; i < e.size(); ++i) {
                  if(t.at(i) - lowestT < conf.agata.windowRight
                     && t.at(i) - lowestT >= conf.agata.windowLeft)
                      idx.push_back(i);
              }
              agataOutput.resize(idx.size());
              for(size_t i{0}; i < idx.size(); ++i) {
                  agataOutput.E[i]              = e.at(idx[i]);
                  agataOutput.CoreID[i]         = core.at(idx[i]);
                  agataOutput.agata_coinc[i]    = false;
                  agataOutput.labr_coinc[i]     = false;
                  agataOutput.prisma_coinc[i]   = false;
                  agataOutput.dante_coinc[i]    = false;
                  agataOutput.spider_coinc[i]   = false;
                  agataOutput.euclides_coinc[i] = false;
                  agataOutput.Time[i] = (t.at(idx[i])) * conf.agata.cfd_unit;
                  agataOutput.Pos[i]  = TVector3(y.at(idx[i]) * UNITS::mm,
                                                 -x.at(idx[i]) * UNITS::mm,
                                                 z.at(idx[i]) * UNITS::mm);
              }
              return idx;
          };

    ULong64_t          TS{std::numeric_limits<ULong64_t>::max()};
    std::vector<float> e;
    std::vector<float> t;
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<int>   core;

    // Histograms before analysis
    UserHistograms&    h = userHistograms;

    // CORE
    e.resize(**agataReader.nbCores);
    t.resize(**agataReader.nbCores);
    x.resize(**agataReader.nbCores);
    y.resize(**agataReader.nbCores);
    z.resize(**agataReader.nbCores);
    core.resize(**agataReader.nbCores);
    for(int i{0}; i < **agataReader.nbCores; ++i) {
        if(TS > agataReader.coreTS->At(i)) { TS = agataReader.coreTS->At(i); }
    }

    for(int i{0}; i < **agataReader.nbCores; ++i) {
        e.at(i) = agataReader.coreE0->At(i);

        t.at(i) = unsignedDiff<double, unsigned long long>(
                      TS, agataReader.coreTS->At(i))
                + agataReader.coreT0->At(i)
                - 39.5; //-39.5 is due to the fixed cfd delay of agata
        core.at(i) = agataReader.coreId->At(i);
    }

    ana(TS, e, t, x, y, z, core);

    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Small variations
        agataOutput.Pos[i] += TVector3(conf.agata.xShift, conf.agata.yShift,
                                       conf.agata.zShift);
        agataOutput.Pos[i].RotateZ(conf.agata.phi);
        agataOutput.Pos[i].RotateX(conf.agata.thetaX);
        agataOutput.Pos[i].RotateY(conf.agata.thetaY);

        // Major variation
        agataOutput.Pos[i].RotateY(conf.agata.angle - UNITS::CONSTANTS::pi);
    }
    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Symmetric loop
        for(unsigned int j{0}; j < i; ++j) {
            Fill(h.m_coreE0_coreE0, agataOutput.E.at(i), agataOutput.E.at(j));
            Fill(h.m_coreE0_coreE0, agataOutput.E.at(j), agataOutput.E.at(i));
        }
    }
    // Core loop
    for(int i{0}; i < **agataReader.nbCores; ++i) {
        Fill(h.h_coreE0, agataReader.coreE0->At(i));
    }

    // TRACKED
    e.resize(**agataReader.nbTrack);
    t.resize(**agataReader.nbTrack);
    x.resize(**agataReader.nbTrack);
    y.resize(**agataReader.nbTrack);
    z.resize(**agataReader.nbTrack);
    core.resize(**agataReader.nbTrack);
    for(int i{0}; i < **agataReader.nbTrack; ++i) {
        e.at(i) = agataReader.trackE->At(i);
        t.at(i) = agataReader.trackT->At(i)
                - 39.5; //-39.5 is due to the fixed cfd delay of agata
        core.at(i) = agataReader.trackCrystalID->At(i);
        x.at(i)    = agataReader.trackX1->At(i);
        y.at(i)    = agataReader.trackY1->At(i);
        z.at(i)    = agataReader.trackZ1->At(i);
    }
    TS = **agataReader.TStrack;

    ana(TS, e, t, x, y, z, core);

    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Small variations
        agataOutput.Pos[i] += TVector3(conf.agata.xShift, conf.agata.yShift,
                                       conf.agata.zShift);
        agataOutput.Pos[i].RotateZ(conf.agata.phi);
        agataOutput.Pos[i].RotateX(conf.agata.thetaX);
        agataOutput.Pos[i].RotateY(conf.agata.thetaY);

        // Major variation
        agataOutput.Pos[i].RotateY(conf.agata.angle - UNITS::CONSTANTS::pi);
    }
    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Symmetric loop
        for(unsigned int j{0}; j < i; ++j) {
            Fill(h.m_trackE_trackE, agataOutput.E.at(i), agataOutput.E.at(j));
            Fill(h.m_trackE_trackE, agataOutput.E.at(j), agataOutput.E.at(i));
        }
    }
    // Tracked loop
    for(int i{0}; i < **agataReader.nbTrack; ++i) {
        Fill(h.h_trackE, agataReader.trackE->At(i));
    }

    // ADDBACK
    e.resize(**agataReader.nbAdd);
    t.resize(**agataReader.nbAdd);
    x.resize(**agataReader.nbAdd);
    y.resize(**agataReader.nbAdd);
    z.resize(**agataReader.nbAdd);
    core.resize(**agataReader.nbAdd);
    for(int i{0}; i < **agataReader.nbAdd; ++i) {
        if(TS > agataReader.AddTS->At(i)) { TS = agataReader.AddTS->At(i); }
    }
    for(int i{0}; i < **agataReader.nbAdd; ++i) {
        e.at(i) = agataReader.AddE->At(i);
        t.at(i) = unsignedDiff<double, unsigned long long>(
            TS, agataReader.AddTS->At(i));
        for(int c{0}; c < **agataReader.nbCores; ++c) {
            if(agataReader.coreId->At(c) == agataReader.AddId->At(i)) {
                t.at(i) += agataReader.coreT0->At(c)
                         - 39.5; //-39.5 is due to the fixed cfd delay of agata
                break;
            }
        }
        core.at(i) = agataReader.AddId->At(i);
        x.at(i)    = agataReader.AddX->At(i);
        y.at(i)    = agataReader.AddY->At(i);
        z.at(i)    = agataReader.AddZ->At(i);
    }
    ana(TS, e, t, x, y, z, core);

    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Small variations
        agataOutput.Pos[i] += TVector3(conf.agata.xShift, conf.agata.yShift,
                                       conf.agata.zShift);
        agataOutput.Pos[i].RotateZ(conf.agata.phi);
        agataOutput.Pos[i].RotateX(conf.agata.thetaX);
        agataOutput.Pos[i].RotateY(conf.agata.thetaY);

        // Major variation
        agataOutput.Pos[i].RotateY(conf.agata.angle - UNITS::CONSTANTS::pi);

        agataOutput.P[i]
            = TLorentzVector(0, 0, agataOutput.E.at(i), agataOutput.E.at(i));
        agataOutput.P[i].SetTheta(agataOutput.Pos.at(i).Theta());
        agataOutput.P[i].SetPhi(agataOutput.Pos.at(i).Phi());
    }
    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        // Symmetric loop
        for(unsigned int j{0}; j < i; ++j) {
            Fill(h.m_AddE_AddE, agataOutput.E.at(i), agataOutput.E.at(j));
            Fill(h.m_AddE_AddE, agataOutput.E.at(j), agataOutput.E.at(i));
        }
    }
    // Addback loop
    for(int i{0}; i < **agataReader.nbAdd; ++i) {
        Fill(h.h_AddE, agataReader.AddE->At(i));
    }

    // User process
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
